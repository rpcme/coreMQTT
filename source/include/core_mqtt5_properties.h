/**
 * @file core_mqtt5_properties.h
 * @brief MQTT 5 Properties API
 *
 * This file provides the API for handling MQTT 5 properties.
 * Properties are key-value pairs that can be attached to most MQTT 5 packets
 * to provide additional metadata and control information.
 */

#ifndef CORE_MQTT5_PROPERTIES_H
#define CORE_MQTT5_PROPERTIES_H

/* Standard includes. */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* MQTT library includes. */
#include "core_mqtt_serializer.h"

/**
 * @brief MQTT 5 Property Identifiers
 *
 * These identifiers are defined in the MQTT 5 specification.
 */
typedef enum MQTT5PropertyType
{
    MQTT5_PROPERTY_PAYLOAD_FORMAT_INDICATOR = 0x01,
    MQTT5_PROPERTY_MESSAGE_EXPIRY_INTERVAL = 0x02,
    MQTT5_PROPERTY_CONTENT_TYPE = 0x03,
    MQTT5_PROPERTY_RESPONSE_TOPIC = 0x08,
    MQTT5_PROPERTY_CORRELATION_DATA = 0x09,
    MQTT5_PROPERTY_SUBSCRIPTION_IDENTIFIER = 0x0B,
    MQTT5_PROPERTY_SESSION_EXPIRY_INTERVAL = 0x11,
    MQTT5_PROPERTY_ASSIGNED_CLIENT_IDENTIFIER = 0x12,
    MQTT5_PROPERTY_SERVER_KEEP_ALIVE = 0x13,
    MQTT5_PROPERTY_AUTHENTICATION_METHOD = 0x15,
    MQTT5_PROPERTY_AUTHENTICATION_DATA = 0x16,
    MQTT5_PROPERTY_REQUEST_PROBLEM_INFORMATION = 0x17,
    MQTT5_PROPERTY_WILL_DELAY_INTERVAL = 0x18,
    MQTT5_PROPERTY_REQUEST_RESPONSE_INFORMATION = 0x19,
    MQTT5_PROPERTY_RESPONSE_INFORMATION = 0x1A,
    MQTT5_PROPERTY_SERVER_REFERENCE = 0x1C,
    MQTT5_PROPERTY_REASON_STRING = 0x1F,
    MQTT5_PROPERTY_RECEIVE_MAXIMUM = 0x21,
    MQTT5_PROPERTY_TOPIC_ALIAS_MAXIMUM = 0x22,
    MQTT5_PROPERTY_TOPIC_ALIAS = 0x23,
    MQTT5_PROPERTY_MAXIMUM_QOS = 0x24,
    MQTT5_PROPERTY_RETAIN_AVAILABLE = 0x25,
    MQTT5_PROPERTY_USER_PROPERTY = 0x26,
    MQTT5_PROPERTY_MAXIMUM_PACKET_SIZE = 0x27,
    MQTT5_PROPERTY_WILDCARD_SUBSCRIPTION_AVAILABLE = 0x28,
    MQTT5_PROPERTY_SUBSCRIPTION_IDENTIFIER_AVAILABLE = 0x29,
    MQTT5_PROPERTY_SHARED_SUBSCRIPTION_AVAILABLE = 0x2A
} MQTT5PropertyType_t;

/**
 * @brief MQTT 5 Property structure
 *
 * Represents a single MQTT 5 property with its type and value.
 */
typedef struct MQTT5Property
{
    MQTT5PropertyType_t type;
    
    union
    {
        uint8_t byte;
        uint16_t twoByteInteger;
        uint32_t fourByteInteger;
        
        struct
        {
            const uint8_t * pData;
            uint16_t length;
        } binaryData;
        
        struct
        {
            const char * pString;
            uint16_t length;
        } utf8String;
        
        struct
        {
            const char * pKey;
            uint16_t keyLength;
            const char * pValue;
            uint16_t valueLength;
        } userProperty;
    } value;
} MQTT5Property_t;

/**
 * @brief MQTT 5 Properties collection
 *
 * Container for multiple MQTT 5 properties.
 * Uses application-provided buffer for zero-copy design.
 */
typedef struct MQTT5Properties
{
    MQTT5Property_t * pProperties;
    size_t count;
    size_t capacity;
} MQTT5Properties_t;

/**
 * @brief Initialize an MQTT 5 properties collection.
 *
 * @param[in] pProperties Pointer to properties structure to initialize.
 * @param[in] pBuffer Application-provided buffer for properties.
 * @param[in] capacity Maximum number of properties the buffer can hold.
 *
 * @return MQTTSuccess if successful, error code otherwise.
 */
MQTTStatus_t MQTT5_InitProperties( MQTT5Properties_t * pProperties,
                                   MQTT5Property_t * pBuffer,
                                   size_t capacity );

/**
 * @brief Add a property to the collection.
 *
 * @param[in] pProperties Pointer to properties collection.
 * @param[in] pProperty Pointer to property to add.
 *
 * @return MQTTSuccess if successful, error code otherwise.
 */
MQTTStatus_t MQTT5_AddProperty( MQTT5Properties_t * pProperties,
                                const MQTT5Property_t * pProperty );

/**
 * @brief Get a property from the collection by type.
 *
 * @param[in] pProperties Pointer to properties collection.
 * @param[in] type Property type to search for.
 * @param[out] pProperty Pointer to store found property.
 *
 * @return MQTTSuccess if found, MQTTBadParameter if not found.
 */
MQTTStatus_t MQTT5_GetProperty( const MQTT5Properties_t * pProperties,
                                MQTT5PropertyType_t type,
                                MQTT5Property_t * pProperty );

/**
 * @brief Calculate the serialized size of properties.
 *
 * @param[in] pProperties Pointer to properties collection.
 *
 * @return Size in bytes of serialized properties.
 */
size_t MQTT5_GetPropertiesSize( const MQTT5Properties_t * pProperties );

/**
 * @brief Serialize properties to a buffer.
 *
 * @param[in] pProperties Pointer to properties collection.
 * @param[out] pBuffer Buffer to write serialized properties.
 * @param[in,out] pSize Input: buffer size, Output: bytes written.
 *
 * @return MQTTSuccess if successful, error code otherwise.
 */
MQTTStatus_t MQTT5_SerializeProperties( const MQTT5Properties_t * pProperties,
                                        uint8_t * pBuffer,
                                        size_t * pSize );

/**
 * @brief Deserialize properties from a buffer.
 *
 * @param[out] pProperties Pointer to properties collection.
 * @param[in] pBuffer Buffer containing serialized properties.
 * @param[in] size Size of buffer.
 *
 * @return MQTTSuccess if successful, error code otherwise.
 */
MQTTStatus_t MQTT5_DeserializeProperties( MQTT5Properties_t * pProperties,
                                          const uint8_t * pBuffer,
                                          size_t size );

/**
 * @brief Decode a Variable Byte Integer from buffer.
 *
 * @param[in] pBuffer Buffer containing VBI.
 * @param[in] bufferSize Size of buffer.
 * @param[out] pValue Decoded value.
 *
 * @return Number of bytes read, or 0 on error.
 */
size_t MQTT5_DecodeVariableByteInteger( const uint8_t * pBuffer,
                                        size_t bufferSize,
                                        uint32_t * pValue );

#endif /* CORE_MQTT5_PROPERTIES_H */
