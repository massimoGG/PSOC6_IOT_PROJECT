/******************************************************************************
* File Name:   mqtt_client_config.h
*
* Description: This file contains all the configuration macros used by the
*              MQTT client in this example.
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2020-2022, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

#ifndef MQTT_CLIENT_CONFIG_H_
#define MQTT_CLIENT_CONFIG_H_

#include "cy_mqtt_api.h"

/*******************************************************************************
* Macros
********************************************************************************/

/***************** MQTT CLIENT CONNECTION CONFIGURATION MACROS *****************/
#define MQTT_BROKER_ADDRESS               "massimog.net"
#define MQTT_PORT                         8883

/* Set this macro to 1 if a secure (TLS) connection to the MQTT Broker is  
 * required to be established, else 0.
 */
#define MQTT_SECURE_CONNECTION            ( 1 )

/* Configure the user credentials to be sent as part of MQTT CONNECT packet */
#define MQTT_USERNAME                     "User"
#define MQTT_PASSWORD                     ""


/********************* MQTT MESSAGE CONFIGURATION MACROS **********************/
/* The MQTT topics to be used by the publisher and subscriber. */
#define MQTT_PUB_TOPIC                    "security"
#define MQTT_SUB_TOPIC                    "security"

/* Set the QoS that is associated with the MQTT publish, and subscribe messages.
 * Valid choices are 0, 1, and 2. Other values should not be used in this macro.
 */
#define MQTT_MESSAGES_QOS                 ( 1 )

/* Configuration for the 'Last Will and Testament (LWT)'. It is an MQTT message 
 * that will be published by the MQTT broker if the MQTT connection is 
 * unexpectedly closed. This configuration is sent to the MQTT broker during 
 * MQTT connect operation and the MQTT broker will publish the Will message on 
 * the Will topic when it recognizes an unexpected disconnection from the client.
 * 
 * If you want to use the last will message, set this macro to 1 and configure
 * the topic and will message, else 0.
 */
#define ENABLE_LWT_MESSAGE                ( 0 )
#if ENABLE_LWT_MESSAGE
    #define MQTT_WILL_TOPIC_NAME          MQTT_PUB_TOPIC "/will"
    #define MQTT_WILL_MESSAGE             ("MQTT client unexpectedly disconnected!")
#endif

/******************* OTHER MQTT CLIENT CONFIGURATION MACROS *******************/
/* A unique client identifier to be used for every MQTT connection. */
#define MQTT_CLIENT_IDENTIFIER            "psoc6-mqtt-client"

/* The timeout in milliseconds for MQTT operations in this example. */
#define MQTT_TIMEOUT_MS                   ( 5000 )

/* The keep-alive interval in seconds used for MQTT ping request. */
#define MQTT_KEEP_ALIVE_SECONDS           ( 60 )

/* Every active MQTT connection must have a unique client identifier. If you 
 * are using the above 'MQTT_CLIENT_IDENTIFIER' as client ID for multiple MQTT 
 * connections simultaneously, set this macro to 1. The device will then
 * generate a unique client identifier by appending a timestamp to the 
 * 'MQTT_CLIENT_IDENTIFIER' string. Example: 'psoc6-mqtt-client5927'
 */
#define GENERATE_UNIQUE_CLIENT_ID         ( 1 )

/* The longest client identifier that an MQTT server must accept (as defined
 * by the MQTT 3.1.1 spec) is 23 characters. However some MQTT brokers support 
 * longer client IDs. Configure this macro as per the MQTT broker specification. 
 */
#define MQTT_CLIENT_IDENTIFIER_MAX_LEN    ( 23 )

/* As per Internet Assigned Numbers Authority (IANA) the port numbers assigned 
 * for MQTT protocol are 1883 for non-secure connections and 8883 for secure
 * connections. In some cases there is a need to use other ports for MQTT like
 * port 443 (which is reserved for HTTPS). Application Layer Protocol 
 * Negotiation (ALPN) is an extension to TLS that allows many protocols to be 
 * used over a secure connection. The ALPN ProtocolNameList specifies the 
 * protocols that the client would like to use to communicate over TLS.
 * 
 * This macro specifies the ALPN Protocol Name to be used that is supported
 * by the MQTT broker in use.
 * Note: For AWS IoT, currently "x-amzn-mqtt-ca" is the only supported ALPN 
 *       ProtocolName and it is only supported on port 443.
 * 
 * Uncomment the below line and specify the ALPN Protocol Name to use this 
 * feature.
 */
// #define MQTT_ALPN_PROTOCOL_NAME           "x-amzn-mqtt-ca"

/* Server Name Indication (SNI) is extension to the Transport Layer Security 
 * (TLS) protocol. As required by some MQTT Brokers, SNI typically includes the 
 * hostname in the Client Hello message sent during TLS handshake.
 * 
 * Uncomment the below line and specify the SNI Host Name to use this extension
 * as specified by the MQTT Broker.
 */
// #define MQTT_SNI_HOSTNAME                 "SNI_HOST_NAME"

/* A Network buffer is allocated for sending and receiving MQTT packets over 
 * the network. Specify the size of this buffer using this macro.
 * 
 * Note: The minimum buffer size is defined by 'CY_MQTT_MIN_NETWORK_BUFFER_SIZE' 
 * macro in the MQTT library. Please ensure this macro value is larger than 
 * 'CY_MQTT_MIN_NETWORK_BUFFER_SIZE'.
 */
#define MQTT_NETWORK_BUFFER_SIZE          ( 2 * CY_MQTT_MIN_NETWORK_BUFFER_SIZE )

/* Maximum MQTT connection re-connection limit. */
#define MAX_MQTT_CONN_RETRIES            (150u)

/* MQTT re-connection time interval in milliseconds. */
#define MQTT_CONN_RETRY_INTERVAL_MS      (2000)


/**************** MQTT CLIENT CERTIFICATE CONFIGURATION MACROS ****************/

/* Configure the below credentials in case of a secure MQTT connection. */
/* PEM-encoded client certificate */
#define CLIENT_CERTIFICATE      \
"-----BEGIN CERTIFICATE-----\n"\
"MIIDoTCCAokCFHc3sM9fJFAHNJxnlc/k83YwrakFMA0GCSqGSIb3DQEBCwUAMIGM\n"\
"MQswCQYDVQQGEwJCRTEQMA4GA1UECAwHTGltYnVyZzEQMA4GA1UEBwwHSGFzc2Vs\n"\
"dDERMA8GA1UECgwIbWFzc2ltb2cxDDAKBgNVBAsMA2lvdDEVMBMGA1UEAwwMbWFz\n"\
"c2ltb2cubmV0MSEwHwYJKoZIhvcNAQkBFhJhZG1pbkBtYXNzaW1vZy5uZXQwHhcN\n"\
"MjIxMjMxMTA0NDA2WhcNMjMxMjI2MTA0NDA2WjCBjDELMAkGA1UEBhMCQkUxEDAO\n"\
"BgNVBAgMB0xpbWJ1cmcxEDAOBgNVBAcMB0hhc3NlbHQxETAPBgNVBAoMCG1hc3Np\n"\
"bW9nMQwwCgYDVQQLDANpb3QxFTATBgNVBAMMDG1hc3NpbW9nLm5ldDEhMB8GCSqG\n"\
"SIb3DQEJARYSYWRtaW5AbWFzc2ltb2cubmV0MIIBIjANBgkqhkiG9w0BAQEFAAOC\n"\
"AQ8AMIIBCgKCAQEAr4f6k2Yko7MUZdnGdedsGxEDSRsaLF3UY1fUwQuI+D1uKZ5O\n"\
"uNQ2VgU7chI6EPq++qUBAl/W01YxI610ZHz6qAlN2O2ZXIkiMwR+t6uDnCujhbeP\n"\
"GKKEja6xMQ+ilXFGwrAdEkSN15OxcXmYEJkIS3k2xN0xCM89wlEWxeZYtYHm7LI0\n"\
"qoPVteNxQHaNLKBvpIYxKT8wzVTYekAq788/tYT+ROQui0qL4xRYdClyIDPqRQbP\n"\
"9CKJd3kJsKTFlRAw6bj86/JAQcZgvUYEGdyGJ8ZZIb3JRTWzsTkldfGn5egVy7/X\n"\
"aCSUMflp8DgzkZ9un03z7hRLYukeMUEsHE5n2wIDAQABMA0GCSqGSIb3DQEBCwUA\n"\
"A4IBAQCXhVq+GKfI1Y02hOcujSkIzbo4Daoqp8eJw1oazeIPQncC6Mjn4e/OResQ\n"\
"2CiqCaWmnibxL1uiEIBcTiKZy4dIRfMvOQAsGCc7U8fKbWdg8hgYRufH8ltQpbMo\n"\
"Z3Sp2LicqLcOaLPHaT2SuCsOG4wbKcomLuZTYgW40FBSdvNNRV3V+meTos9cCeBE\n"\
"jEO8rwwaM8PmY0Mx2PFNRp8JVO1JRrTtysF9msKOlE3cQo3/UHTZZ7KIsQgAgRIf\n"\
"zazrjiLXIhLHKmX09/qgX6Nw5N8V6CPs8dsV4zlRQc67mzDyHaIAKPhHYnMixOUl\n"\
"vlWlrX6ph2ZpyIt6GrTaoEWDonbN\n"\
"-----END CERTIFICATE-----"

/* PEM-encoded client private key */
#define CLIENT_PRIVATE_KEY          \
"-----BEGIN RSA PRIVATE KEY-----\n"\
"MIIEogIBAAKCAQEAr4f6k2Yko7MUZdnGdedsGxEDSRsaLF3UY1fUwQuI+D1uKZ5O\n"\
"uNQ2VgU7chI6EPq++qUBAl/W01YxI610ZHz6qAlN2O2ZXIkiMwR+t6uDnCujhbeP\n"\
"GKKEja6xMQ+ilXFGwrAdEkSN15OxcXmYEJkIS3k2xN0xCM89wlEWxeZYtYHm7LI0\n"\
"qoPVteNxQHaNLKBvpIYxKT8wzVTYekAq788/tYT+ROQui0qL4xRYdClyIDPqRQbP\n"\
"9CKJd3kJsKTFlRAw6bj86/JAQcZgvUYEGdyGJ8ZZIb3JRTWzsTkldfGn5egVy7/X\n"\
"aCSUMflp8DgzkZ9un03z7hRLYukeMUEsHE5n2wIDAQABAoIBAGJ22dYzdxwpCak0\n"\
"p4JeJiNswLYSsPTwUYLEs0DzEouGEjTga/J8gFpypu0PUmlR7y20v8MwNdbpQ7lq\n"\
"xJ7WCNmYFoY4ik4BUyDhRaAMY1CInjsVQ/0itU6xDw6hYaeJGpN9rWEg+ZxTajdo\n"\
"aSfuYFvODC4UkjwJ3PATAr5Sv7JY1FYW21Yug0oPy5bPsYOZKMQ1/+vrGArx5Zec\n"\
"saFBKCKfUV651CWydKiV6BXmunYS+1wDGBijkkvhwj3xIqrI17CfvLRsL+daL73Y\n"\
"lQAxQk41A3qZ3E4HdJiUaqYLHjMyW4tyYXRvkInx/sqQQ/Cx/Ad6D2okdlgSM43x\n"\
"dEVPI0ECgYEA6bz0X54tPoZcm2lBPxbjzmOCc++SBd5OTJR4jPK79VVE6GWuTkDi\n"\
"2tki5YAfo77iwfUNarGGGcLzxlCMwi/aGo3e2vQGqDa6LkN6yQNeFa+j5mPaokdJ\n"\
"nz7Yld2RLpKhNrkBmSjiW/6aEDueQSzaJjoacD+C1Sq/umWe0v5f7bsCgYEAwD/P\n"\
"3yp6ODof5V8+4nKckwLPoxzXIxs+jqwnlGvKNm1RVYoCp0F3diJVGzbXNrHxz1Uh\n"\
"ihzvC4nnRh7/BFYDaRULQ9hJJ06SgkqbVjZdOEjJj59F9yHiLGMbKS3yr3GQdOwx\n"\
"j03FQJhaj7Ql+QtIbcIkiycA0a55oEskp7AgvGECgYAOSZSOe7a8Sa5Dea2ZU6bd\n"\
"KMwCljRQ04PpvnGX/AZtOqcZpY2ADwtO4VL2wFwuwC5QONB3Ix7VWEtaZfYUsKQe\n"\
"V2m17JCzm5TJ2yoYO3J21OOp45SrsrvXVhobHw4O0Lu2Ap0MZalggjGcqGhGB53W\n"\
"FFslXVx79z78/9UzhIUMnQKBgEy1qZYcduQUnUGCva7RmJQ4XO1yxo0lg9Kd8rUQ\n"\
"RYFuITQGe4SNZMNoc5kGSt1yJ1BZYaagYOmzvuqF16+9OmMlmHcbyCS8/0ypxE2k\n"\
"J5IbyQFbxikUBhE8AkxuHxXONJvS2vuZRom9wPTyJuy2idHx028KwxLnkXRaR2f5\n"\
"qTZBAoGAb+VtN7eaVT06fc/aCj+rZol3sCBCxR+0evVeptq8trzq2bXZFNx/I/TV\n"\
"kyiWvGW6QioYiUXafIGJJMHIWDt7ix5xyJHARxYnzM3KkEqKQzST2hk/ANxeAaOz\n"\
"O8jDB2GgyfdndpfLbSM/HvhxAIOOsyhqcLgcuRMPFGPoN7+lezo=\n"\
"-----END RSA PRIVATE KEY-----"

/* PEM-encoded Root CA certificate */
#define ROOT_CA_CERTIFICATE     \
"-----BEGIN CERTIFICATE-----\n"\
"MIID+zCCAuOgAwIBAgIUQ92X6LPT1+/8Wbb1iF3ErufoxuswDQYJKoZIhvcNAQEL\n"\
"BQAwgYwxCzAJBgNVBAYTAkJFMRAwDgYDVQQIDAdMaW1idXJnMRAwDgYDVQQHDAdI\n"\
"YXNzZWx0MREwDwYDVQQKDAhtYXNzaW1vZzEMMAoGA1UECwwDaW90MRUwEwYDVQQD\n"\
"DAxtYXNzaW1vZy5uZXQxITAfBgkqhkiG9w0BCQEWEmFkbWluQG1hc3NpbW9nLm5l\n"\
"dDAeFw0yMjEyMzExMDI2MjJaFw0yNzEyMzExMDI2MjJaMIGMMQswCQYDVQQGEwJC\n"\
"RTEQMA4GA1UECAwHTGltYnVyZzEQMA4GA1UEBwwHSGFzc2VsdDERMA8GA1UECgwI\n"\
"bWFzc2ltb2cxDDAKBgNVBAsMA2lvdDEVMBMGA1UEAwwMbWFzc2ltb2cubmV0MSEw\n"\
"HwYJKoZIhvcNAQkBFhJhZG1pbkBtYXNzaW1vZy5uZXQwggEiMA0GCSqGSIb3DQEB\n"\
"AQUAA4IBDwAwggEKAoIBAQCrqpD8CMTlhsteOQtqO2Ux1GaVhFjoEPu13PYbPazv\n"\
"Cesq8T0tos65baq8fVPf7L9ARQGaCt0Thgm5YZOhMNEZC8iGin3oMXFAU+2ygCiE\n"\
"Xug77aqJu0pkr9W0v0k6v4/YotvUfhUjoxny8FMcY6CLUJRROuQrNh0NM6kC1D1z\n"\
"te8uUvSz3jYUoTsR+8ehPdViUJAspd4D23ltAvRUwpDMQ1KnDOSBg5B+JStfPzED\n"\
"XDgaBKvzR1Lj4ProC2k4Tdm3AiFIMvZJ9RFJ+yIaFCUkrMfELFfrTaavIycAgji/\n"\
"zCLT13nHXBayl9a2STrFRQohsAWCohP2hIP10oSFA2hrAgMBAAGjUzBRMB0GA1Ud\n"\
"DgQWBBR1TE5T4H7lVWDD55xbFHhRj+EGFzAfBgNVHSMEGDAWgBR1TE5T4H7lVWDD\n"\
"55xbFHhRj+EGFzAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQCI\n"\
"x86kmDlUw6sQ83kYrb6OIvgx2N7H2au2M3Bj9d7dtI3V0JBoVXpPLCfkkLfynZmf\n"\
"ngUW9cHne/tlO4+m4u+ajLMo+pJpN8fkCpotG1ua9IfV8apTGS8WDWVyMERc7OjY\n"\
"l0LSDPXFTPlEq0hZX7tr/BnsmWIu0vY15ZBLDRqcRDmb3TYxBSk4ml5wEF/RvtMY\n"\
"niBVUKu7Lsd6sdRkL1R//YZNigQCqmaNm1GxXpiYm5jq6FlKT/T8hhpqvx+XLG9z\n"\
"2oGMnGwHQNZNrTtU2i07z9IM2dHh9pWOI/tcuyMb12s1Ly9Z6hitDEXXwLw23Nrn\n"\
"1CCqKd5F6Shxbka4p04v\n"\
"-----END CERTIFICATE-----"


/******************************************************************************
* Global Variables
*******************************************************************************/
extern cy_mqtt_broker_info_t broker_info;
extern cy_awsport_ssl_credentials_t  *security_info;
extern cy_mqtt_connect_info_t connection_info;


#endif /* MQTT_CLIENT_CONFIG_H_ */
