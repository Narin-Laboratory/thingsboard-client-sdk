#ifndef Provision_Callback_h
#define Provision_Callback_h

// Local includes.
#include "Callback_Watchdog.h"


// Struct dispatch tags, to differentiate between constructors, allows the same paramter types to be passed
struct Access_Token{};
struct Device_Access_Token{};
struct Basic_MQTT_Credentials{};
struct X509_Certificate{};


/// @brief Provisioning callback wrapper,
/// contains the needed configuration settings to create the request that should be sent to the server.
/// Documentation about the specific use of Provisioning devices in ThingsBoard can be found here https://thingsboard.io/docs/user-guide/device-provisioning/
class Provision_Callback : public Callback<void, JsonDocument const &> {
  public:
    /// @brief Constructs empty callback, will result in never being called. Internals are simply default constructed as nullptr
    Provision_Callback() = default;

    /// @brief Constructs callback that will be fired upon a provision request arrival,
    /// where the requested credentials were sent by the cloud and received by the client.
    /// Using the credentials generated by the ThingsBoard server method. See https://thingsboard.io/docs/user-guide/device-provisioning/?mqttprovisioning=without#mqtt-device-apis
    /// @param callback Callback method that will be called upon data arrival with the given data that was received serialized into a JsonDocument
    /// @param provision_device_key Device profile provisioning key of the device profile that should be used to create the device under
    /// @param provision_device_secret Device profile provisioning secret of the device profile that should be used to create the device under
    /// @param device_name Name the created device should have on the cloud,
    /// pass nullptr or an empty string if a random string should be used as a name instead
    Provision_Callback(Access_Token, function callback, char const * provision_device_key, char const * provision_device_secret, char const * device_name = nullptr, uint64_t const & timeout_microseconds = 0U, Callback_Watchdog::function timeout_callback = nullptr);

    /// @brief Constructs callback that will be fired upon a provision request arrival,
    /// where the requested credentials were sent by the cloud and received by the client.
    /// Using the device supplies access token method. See https://thingsboard.io/docs/user-guide/device-provisioning/?mqttprovisioning=without#mqtt-device-apis
    /// @param callback Callback method that will be called upon data arrival with the given data that was received serialized into a JsonDocument
    /// @param provision_device_key Device profile provisioning key of the device profile that should be used to create the device under
    /// @param provision_device_secret Device profile provisioning secret of the device profile that should be used to create the device under
    /// @param access_token Access token generated by the device, that will be used by the provisioned device, alternative to letting the access token be generated by the cloud instead
    /// @param device_name Name the created device should have on the cloud,
    /// pass nullptr or an empty string if a random string should be used as a name instead
    /// @param timeout_microseconds Optional amount of microseconds until we expect to have received a response and if we didn't, we call the previously subscribed callback.
    /// If the value is 0 we will not start the timer and therefore never call the timeout callback method, default = 0
    /// @param timeout_callback Optional callback method that will be called upon request timeout (did not receive a response in the given timeout time). Can happen if the requested method does not exist on the cloud,
    /// or if the connection could not be established, default = nullptr
    Provision_Callback(Device_Access_Token, function callback, char const * provision_device_key, char const * provision_device_secret, char const * access_token, char const * device_name = nullptr, uint64_t const & timeout_microseconds = 0U, Callback_Watchdog::function timeout_callback = nullptr);

    /// @brief Constructs callback that will be fired upon a provision request arrival,
    /// where the requested credentials were sent by the cloud and received by the client.
    /// Using the device supplies basic MQTT credentials method. See https://thingsboard.io/docs/user-guide/device-provisioning/?mqttprovisioning=without#mqtt-device-apis
    /// @param callback Callback method that will be called upon data arrival with the given data that was received serialized into a JsonDocument
    /// @param provision_device_key Device profile provisioning key of the device profile that should be used to create the device under
    /// @param provision_device_secret Device profile provisioning secret of the device profile that should be used to create the device under
    /// @param username Basic MQTT credentials username, that will be used by the provisioned device
    /// @param password Basic MQTT credentials password, that will be used by the provisioned device
    /// @param client_id Basic MQTT credentials client_id, that will be used by the provisioned device
    /// @param device_name Name the created device should have on the cloud,
    /// pass nullptr or an empty string if a random string should be used as a name instead
    /// @param timeout_microseconds Optional amount of microseconds until we expect to have received a response and if we didn't, we call the previously subscribed callback.
    /// If the value is 0 we will not start the timer and therefore never call the timeout callback method, default = 0
    /// @param timeout_callback Optional callback method that will be called upon request timeout (did not receive a response in the given timeout time). Can happen if the requested method does not exist on the cloud,
    /// or if the connection could not be established, default = nullptr
    Provision_Callback(Basic_MQTT_Credentials, function callback, char const * provision_device_key, char const * provision_device_secret, char const * username, char const * password, char const * client_id, char const * device_name = nullptr, uint64_t const & timeout_microseconds = 0U, Callback_Watchdog::function timeout_callback = nullptr);

    /// @brief Constructs callback that will be fired upon a provision request arrival,
    /// where the requested credentials were sent by the cloud and received by the client.
    /// Using the device supplies X.509 certificate method. See https://thingsboard.io/docs/user-guide/device-provisioning/?mqttprovisioning=without#mqtt-device-apis
    /// @param callback Callback method that will be called upon data arrival with the given data that was received serialized into a JsonDocument
    /// @param provision_device_key Device profile provisioning key of the device profile that should be used to create the device under
    /// @param provision_device_secret Device profile provisioning secret of the device profile that should be used to create the device under
    /// @param hash Public X.509 certificate hash, that will be used by the provisioned device
    /// @param device_name Name the created device should have on the cloud,
    /// pass nullptr or an empty string if a random string should be used as a name instead
    /// @param timeout_microseconds Optional amount of microseconds until we expect to have received a response and if we didn't, we call the previously subscribed callback.
    /// If the value is 0 we will not start the timer and therefore never call the timeout callback method, default = 0
    /// @param timeout_callback Optional callback method that will be called upon request timeout (did not receive a response in the given timeout time). Can happen if the requested method does not exist on the cloud,
    /// or if the connection could not be established, default = nullptr
    Provision_Callback(X509_Certificate, function callback, char const * provision_device_key, char const * provision_device_secret, char const * hash, char const * device_name = nullptr, uint64_t const & timeout_microseconds = 0U, Callback_Watchdog::function timeout_callback = nullptr);

    ~Provision_Callback() override = default;

    /// @brief Gets the device profile provisioning key of the device profile,
    /// that should be used to create the device under
    /// @return Device profile provisioning key
    char const * Get_Device_Key() const;

    /// @brief Sets the device profile provisioning key of the device profile,
    /// that should be used to create the device under
    /// @param provision_device_key Device profile provisioning key
    void Set_Device_Key(char const * provision_device_key);

    /// @brief Gets the device profile provisioning secret of the device profile,
    /// that should be used to create the device under
    /// @return Device profile provisioning secret
    char const * Get_Device_Secret() const;

    /// @brief Gets the device profile provisioning secret of the device profile,
    /// that should be used to create the device under
    /// @param provision_device_secret Device profile provisioning secret
    void Set_Device_Secret(char const * provision_device_secret);

    /// @brief Gets the name the created device should have on the cloud,
    /// is a nullptr or an empty string if a random string should be used as a name instead
    /// @return Name the created device should have on the cloud
    char const * Get_Device_Name() const;

    /// @brief Sets the name the created device should have on the cloud,
    /// is a nullptr or an empty string if a random string should be used as a name instead
    /// @param device_name Name the created device should have on the cloud
    void Set_Device_Name(char const * device_name) ;

    /// @brief Gets the access token generated by the device,
    /// that will be used by the provisioned device,
    /// alternative to letting the access token be generated by the cloud instead
    /// @return Access token generated by the device
    char const * Get_Device_Access_Token() const;

    /// @brief Sets the access token generated by the device,
    /// that will be used by the provisioned device,
    /// alternative to letting the access token be generated by the cloud instead
    /// @param access_token Access token generated by the device
    void Set_Device_Access_Token(char const * access_token);

    /// @brief Gets the basic MQTT credentials username, that will be used by the provisioned device
    /// @return Basic MQTT credentials username
    char const * Get_Credentials_Username() const;

    /// @brief Sets the basic MQTT credentials username, that will be used by the provisioned device
    /// @param username Basic MQTT credentials username
    void Set_Credentials_Username(char const * username);

    /// @brief Gets the basic MQTT credentials password, that will be used by the provisioned device
    /// @return Basic MQTT credentials password
    char const * Get_Credentials_Password() const;

    /// @brief Sets the basic MQTT credentials password, that will be used by the provisioned device
    /// @param password Basic MQTT credentials password
    void Set_Credentials_Password(char const * password);

    /// @brief Gets the basic MQTT credentials client_id, that will be used by the provisioned device
    /// @return Basic MQTT credentials client_id
    char const * Get_Credentials_Client_ID() const;

    /// @brief Sets the basic MQTT credentials client_id, that will be used by the provisioned device
    /// @param client_id Basic MQTT credentials client_id
    void Set_Credentials_Client_ID(char const * client_id);

    /// @brief Gets the public X.509 certificate hash, that will be used by the provisioned device
    /// @return Public X.509 certificate hash
    char const * Get_Certificate_Hash() const;

    /// @brief Sets the public X.509 certificate hash, that will be used by the provisioned device
    /// @param hash Public X.509 certificate hash
    void Set_Certificate_Hash(char const * hash);

    /// @brief Gets the string containing the used credentials type that decides which provisioning method is actually used,
    /// by the Provision_Callback and therefore decides what response we will receive from the server
    /// @return String containing the used credentials type
    char const * Get_Credentials_Type() const;

    /// @brief Gets the amount of microseconds until we expect to have received a response
    /// @return Timeout time until timeout callback is called
    uint64_t const & Get_Timeout() const;

    /// @brief Sets the amount of microseconds until we expect to have received a response
    /// @param timeout_microseconds Timeout time until timeout callback is called
    void Set_Timeout(uint64_t const & timeout_microseconds);

#if !THINGSBOARD_USE_ESP_TIMER
    /// @brief Updates the internal timeout timer
    void Update_Timeout_Timer();
#endif // !THINGSBOARD_USE_ESP_TIMER

    /// @brief Starts the internal timeout timer if we actually received a configured valid timeout time and a valid callback.
    /// Is called as soon as the request is actually sent
    void Start_Timeout_Timer();

    /// @brief Stops the internal timeout timer, is called as soon as an answer is received from the cloud
    /// if it isn't we call the previously subscribed callback instead
    void Stop_Timeout_Timer();

    /// @brief Sets the callback method that will be called upon request timeout (did not receive a response in the given timeout time)
    /// @param timeout_callback Callback function that will be called
    void Set_Timeout_Callback(Callback_Watchdog::function timeout_callback);

  private:
    char const        *m_device_key = {};          // Device profile provisioning key
    char const        *m_device_secret = {};       // Device profile provisioning secret
    char const        *m_device_name = {};         // Device name the provisioned device should have
    char const        *m_access_token = {};        // Access token supplied by the device, if it should not be generated by the server instead
    char const        *m_cred_username = {};       // MQTT credential username, if the MQTT basic credentials method is used
    char const        *m_cred_password = {};       // MQTT credential password, if the MQTT basic credentials method is used
    char const        *m_cred_client_id = {};      // MQTT credential client_id, if Mthe QTT basic credentials method is used
    char const        *m_hash = {};                // X.509 certificate hash, if the X.509 certificate authentication method is used
    char const        *m_credentials_type = {};    // Credentials type we are requesting from the server, nullptr for the default option (Credentials generated by the ThingsBoard server)
    uint64_t          m_timeout_microseconds = {}; // Timeout time until we expect response to request
    Callback_Watchdog m_timeout_callback = {};     // Handles callback that will be called if request times out
};

#endif // Provision_Callback_h
