#ifndef Client_Side_RPC_h
#define Client_Side_RPC_h

// Local includes.
#include "RPC_Request_Callback.h"
#include "IAPI_Implementation.h"


// Client side RPC topics.
char constexpr RPC_RESPONSE_SUBSCRIBE_TOPIC[] = "v1/devices/me/rpc/response/+";
char constexpr RPC_RESPONSE_TOPIC[] = "v1/devices/me/rpc/response/";
char constexpr RPC_SEND_REQUEST_TOPIC[] = "v1/devices/me/rpc/request/%u";
// Log messages.
char constexpr CLIENT_RPC_METHOD_NULL[] = "Client-side RPC method name is NULL";
#if !THINGSBOARD_ENABLE_DYNAMIC
char constexpr RPC_REQUEST_OVERFLOWED[] = "Client-side RPC request overflowed, increase MaxRequestRPC (%u)";
char constexpr CLIENT_SIDE_RPC_SUBSCRIPTIONS[] = "client-side RPC";
#endif // !THINGSBOARD_ENABLE_DYNAMIC
char constexpr RPC_EMPTY_PARAMS_VALUE[] = "{}";


/// @brief Handles the internal implementation of the ThingsBoard client side RPC API.
/// See https://thingsboard.io/docs/user-guide/rpc/#client-side-rpc for more information
/// @tparam Logger Implementation that should be used to print error messages generated by internal processes and additional debugging messages if THINGSBOARD_ENABLE_DEBUG is set, default = DefaultLogger
#if THINGSBOARD_ENABLE_DYNAMIC
template <typename Logger = DefaultLogger>
#else
/// @tparam MaxSubscriptions Maximum amount of simultaneous client side rpc requests.
/// Once the maximum amount has been reached it is not possible to increase the size, this is done because it allows to allcoate the memory on the stack instead of the heap, default = Default_Subscriptions_Amount (1)
/// @tparam MaxRequestRPC Maximum amount of key-value pairs that will ever be sent as parameters to the requests client side rpc method, allows to use a StaticJsonDocument on the stack in the background.
/// Is expected to only request client side rpc requests, that do not additionally send any parameters. If we attempt to send parameters, we have to adjust the size accordingly.
/// Default value is big enough to hold no parameters, but simply the default method name and params key needed for the request, if additional parameters are sent with the request the size has to be increased by one for each key-value pair.
/// See https://arduinojson.org/v6/assistant/ for more information on how to estimate the required size and divide the result by 16 and add 2 to receive the required MaxRequestRPC value, default = Default_Request_RPC_Amount (2)
template<size_t MaxSubscriptions = Default_Subscriptions_Amount, size_t MaxRequestRPC = Default_Request_RPC_Amount, typename Logger = DefaultLogger>
#endif // THINGSBOARD_ENABLE_DYNAMIC
class Client_Side_RPC : public IAPI_Implementation {
  public:
    /// @brief Constructor
    Client_Side_RPC() = default;

    /// @brief Requests one client-side RPC callback,
    /// that will be called if a response from the server for the method with the given name is received.
    /// Because the client-side RPC request is a single event subscription, meaning we only ever receive a response to our request once,
    /// we automatically unsubscribe and delete the internal allocated data for the request as soon as the response has been received and handled by the subscribed callback.
    /// See https://thingsboard.io/docs/user-guide/rpc/#client-side-rpc for more information
    /// @param callback Callback method that will be called
    /// @return Whether requesting the given callback was successful or not
    bool RPC_Request(RPC_Request_Callback const & callback) {
        char const * method_name = callback.Get_Name();

        if (Helper::stringIsNullorEmpty(method_name)) {
            Logger::printfln(CLIENT_RPC_METHOD_NULL);
            return false;
        }
        RPC_Request_Callback * registered_callback = nullptr;
        if (!RPC_Request_Subscribe(callback, registered_callback)) {
            return false;
        }
        else if (registered_callback == nullptr) {
            return false;
        }

        JsonArray const * parameters = callback.Get_Parameters();

#if THINGSBOARD_ENABLE_DYNAMIC
        // String are const char* and therefore stored as a pointer --> zero copy, meaning the size for the strings is 0 bytes,
        // Data structure size depends on the amount of key value pairs passed + the default method name and params key needed for the request.
        // See https://arduinojson.org/v6/assistant/ for more information on the needed size for the JsonDocument
        TBJsonDocument request_buffer(JSON_OBJECT_SIZE(parameters != nullptr ? parameters->size() + 2U : 2U));
#else
        // Ensure to have enough size for the infinite amount of possible parameters that could be sent to the cloud
        StaticJsonDocument<JSON_OBJECT_SIZE(MaxRequestRPC)> request_buffer;
#endif // THINGSBOARD_ENABLE_DYNAMIC

        request_buffer[RPC_METHOD_KEY] = method_name;

        if (parameters != nullptr && !parameters->isNull()) {
            request_buffer[RPC_PARAMS_KEY] = *parameters;
        }
        else {
            request_buffer[RPC_PARAMS_KEY] = RPC_EMPTY_PARAMS_VALUE;
        }

#if !THINGSBOARD_ENABLE_DYNAMIC
        if (request_buffer.overflowed()) {
            Logger::printfln(RPC_REQUEST_OVERFLOWED, MaxRequestRPC);
            return false;
        }
#endif // !THINGSBOARD_ENABLE_DYNAMIC

        size_t * p_request_id = m_get_request_id_callback.Call_Callback();
        if (p_request_id == nullptr) {
            Logger::printfln(REQUEST_ID_NULL);
            return false;
        }
        auto & request_id = *p_request_id;

        registered_callback->Set_Request_ID(++request_id);
        registered_callback->Start_Timeout_Timer();

        char topic[Helper::detectSize(RPC_SEND_REQUEST_TOPIC, request_id)] = {};
        (void)snprintf(topic, sizeof(topic), RPC_SEND_REQUEST_TOPIC, request_id);
        return m_send_json_callback.Call_Callback(topic, request_buffer, Helper::Measure_Json(request_buffer));
    }

    API_Process_Type Get_Process_Type() const override {
        return API_Process_Type::JSON;
    }

    void Process_Response(char const * topic, uint8_t * payload, unsigned int length) override {
        // Nothing to do
    }

    void Process_Json_Response(char const * topic, JsonDocument const & data) override {
        size_t const request_id = Helper::parseRequestId(RPC_RESPONSE_TOPIC, topic);

#if THINGSBOARD_ENABLE_STL
        auto it = std::find_if(m_rpc_request_callbacks.begin(), m_rpc_request_callbacks.end(), [&request_id](RPC_Request_Callback & rpc_request) {
            return rpc_request.Get_Request_ID() == request_id;
        });
        if (it != m_rpc_request_callbacks.end()) {
            auto & rpc_request = *it;
#else
        for (auto it = m_rpc_request_callbacks.begin(); it != m_rpc_request_callbacks.end(); ++it) {
            auto & rpc_request = *it;

            if (rpc_request.Get_Request_ID() != request_id) {
                continue;
            }
#endif // THINGSBOARD_ENABLE_STL
            rpc_request.Stop_Timeout_Timer();
            rpc_request.Call_Callback(data);

            // Delete callback because the changes have been requested and the callback is no longer needed
            Helper::remove(m_rpc_request_callbacks, it);
#if !THINGSBOARD_ENABLE_STL
            break;
#endif // !THINGSBOARD_ENABLE_STL
        }

        // Attempt to unsubscribe from the shared attribute request topic,
        // if we are not waiting for any further responses with shared attributes from the server.
        // Will be resubscribed if another request is sent anyway
        if (m_rpc_request_callbacks.empty()) {
            (void)RPC_Request_Unsubscribe();
        }
    }

    bool Compare_Response_Topic(char const * topic) const override {
        return strncmp(RPC_RESPONSE_TOPIC, topic, strlen(RPC_RESPONSE_TOPIC)) == 0;
    }

    bool Unsubscribe() override {
        return RPC_Request_Unsubscribe();
    }

    bool Resubscribe_Topic() override {
        m_rpc_request_callbacks.clear();
        return Unsubscribe();
    }

#if !THINGSBOARD_USE_ESP_TIMER
    void loop() override {
        for (auto & rpc_request : m_rpc_request_callbacks) {
            rpc_request.Update_Timeout_Timer();
        }
    }
#endif // !THINGSBOARD_USE_ESP_TIMER

    void Initialize() override {
        // Nothing to do
    }

    void Set_Client_Callbacks(Callback<void, IAPI_Implementation &>::function subscribe_api_callback, Callback<bool, char const * const, JsonDocument const &, size_t const &>::function send_json_callback, Callback<bool, char const * const, char const * const>::function send_json_string_callback, Callback<bool, char const * const>::function subscribe_topic_callback, Callback<bool, char const * const>::function unsubscribe_topic_callback, Callback<uint16_t>::function get_receive_size_callback, Callback<uint16_t>::function get_send_size_callback, Callback<bool, uint16_t, uint16_t>::function set_buffer_size_callback, Callback<size_t *>::function get_request_id_callback) override {
        m_send_json_callback.Set_Callback(send_json_callback);
        m_subscribe_topic_callback.Set_Callback(subscribe_topic_callback);
        m_unsubscribe_topic_callback.Set_Callback(unsubscribe_topic_callback);
        m_get_request_id_callback.Set_Callback(get_request_id_callback);
    }

  private:
    /// @brief Subscribes to the client-side RPC response topic,
    /// that will be called if a reponse from the server for the method with the given name is received.
    /// See https://thingsboard.io/docs/user-guide/rpc/#client-side-rpc for more information
    /// @param callback Callback method that will be called
    /// @param registered_callback Editable pointer to a reference of the local version that was copied from the passed callback
    /// @return Whether requesting the given callback was successful or not
    bool RPC_Request_Subscribe(RPC_Request_Callback const & callback, RPC_Request_Callback * & registered_callback) {
#if !THINGSBOARD_ENABLE_DYNAMIC
        if (m_rpc_request_callbacks.size() + 1 > m_rpc_request_callbacks.capacity()) {
            Logger::printfln(MAX_SUBSCRIPTIONS_EXCEEDED, MAX_SUBSCRIPTIONS_TEMPLATE_NAME, CLIENT_SIDE_RPC_SUBSCRIPTIONS);
            return false;
        }
#endif // !THINGSBOARD_ENABLE_DYNAMIC
        if (!m_subscribe_topic_callback.Call_Callback(RPC_RESPONSE_SUBSCRIBE_TOPIC)) {
            Logger::printfln(SUBSCRIBE_TOPIC_FAILED, RPC_RESPONSE_SUBSCRIBE_TOPIC);
            return false;
        }
        m_rpc_request_callbacks.push_back(callback);
        registered_callback = &m_rpc_request_callbacks.back();
        return true;
    }

    /// @brief Unsubscribes all client-side RPC request callbacks
    /// @return Whether unsubcribing the previously subscribed callbacks
    /// and from the client-side RPC response topic, was successful or not
    bool RPC_Request_Unsubscribe() {
        (void)Resubscribe_Topic();
        return m_unsubscribe_topic_callback.Call_Callback(RPC_RESPONSE_SUBSCRIBE_TOPIC);
    }

    Callback<bool, char const * const, JsonDocument const &, size_t const &> m_send_json_callback = {};          // Send json document callback
    Callback<bool, char const * const>                                       m_subscribe_topic_callback = {};    // Subscribe mqtt topic client callback
    Callback<bool, char const * const>                                       m_unsubscribe_topic_callback = {};  // Unubscribe mqtt topic client callback
    Callback<size_t *>                                                       m_get_request_id_callback = {};     // Get internal request id callback

    // Vectors or array (depends on wheter if THINGSBOARD_ENABLE_DYNAMIC is set to 1 or 0), hold copy of the actual passed data, this is to ensure they stay valid,
    // even if the user only temporarily created the object before the method was called.
    // This can be done because all Callback methods mostly consists of pointers to actual object so copying them
    // does not require a huge memory overhead and is acceptable especially in comparsion to possible problems that could
    // arise if references were used and the end user does not take care to ensure the Callbacks live on for the entirety
    // of its usage, which will lead to dangling references and undefined behaviour.
    // Therefore copy-by-value has been choosen as for this specific use case it is more advantageous,
    // especially because at most we copy internal vectors or array, that will only ever contain a few pointers
#if THINGSBOARD_ENABLE_DYNAMIC
    Vector<RPC_Request_Callback>                                             m_rpc_request_callbacks = {};       // Server side RPC callbacks vector
#else
    Array<RPC_Request_Callback, MaxSubscriptions>                            m_rpc_request_callbacks = {};       // Server side RPC callbacks array
#endif // THINGSBOARD_ENABLE_DYNAMIC
};

#endif // Client_Side_RPC_h
