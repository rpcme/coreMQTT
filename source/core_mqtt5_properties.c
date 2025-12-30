/**
 * @file core_mqtt5_properties.c
 * @brief MQTT 5 Properties Implementation
 */

#include "core_mqtt5_properties.h"
#include <string.h>

/*-----------------------------------------------------------*/

MQTTStatus_t MQTT5_InitProperties( MQTT5Properties_t * pProperties,
                                   MQTT5Property_t * pBuffer,
                                   size_t capacity )
{
    MQTTStatus_t status = MQTTSuccess;

    if( ( pProperties == NULL ) || ( pBuffer == NULL ) || ( capacity == 0U ) )
    {
        status = MQTTBadParameter;
    }
    else
    {
        pProperties->pProperties = pBuffer;
        pProperties->count = 0U;
        pProperties->capacity = capacity;
    }

    return status;
}

/*-----------------------------------------------------------*/

MQTTStatus_t MQTT5_AddProperty( MQTT5Properties_t * pProperties,
                                const MQTT5Property_t * pProperty )
{
    MQTTStatus_t status = MQTTSuccess;

    if( ( pProperties == NULL ) || ( pProperty == NULL ) )
    {
        status = MQTTBadParameter;
    }
    else if( pProperties->count >= pProperties->capacity )
    {
        status = MQTTNoMemory;
    }
    else
    {
        /* Copy property to buffer */
        pProperties->pProperties[ pProperties->count ] = *pProperty;
        pProperties->count++;
    }

    return status;
}

/*-----------------------------------------------------------*/

MQTTStatus_t MQTT5_GetProperty( const MQTT5Properties_t * pProperties,
                                MQTT5PropertyType_t type,
                                MQTT5Property_t * pProperty )
{
    MQTTStatus_t status = MQTTBadParameter;
    size_t i;

    if( ( pProperties == NULL ) || ( pProperty == NULL ) )
    {
        status = MQTTBadParameter;
    }
    else
    {
        /* Search for property by type */
        for( i = 0U; i < pProperties->count; i++ )
        {
            if( pProperties->pProperties[ i ].type == type )
            {
                *pProperty = pProperties->pProperties[ i ];
                status = MQTTSuccess;
                break;
            }
        }
    }

    return status;
}

/*-----------------------------------------------------------*/

size_t MQTT5_GetPropertiesSize( const MQTT5Properties_t * pProperties )
{
    size_t totalSize = 0U;
    size_t i;

    if( pProperties != NULL )
    {
        /* Calculate size of all properties */
        for( i = 0U; i < pProperties->count; i++ )
        {
            const MQTT5Property_t * pProp = &pProperties->pProperties[ i ];
            
            /* Property ID (1 byte) */
            totalSize += 1U;
            
            /* Property value size depends on type */
            switch( pProp->type )
            {
                case MQTT5_PROPERTY_PAYLOAD_FORMAT_INDICATOR:
                case MQTT5_PROPERTY_REQUEST_PROBLEM_INFORMATION:
                case MQTT5_PROPERTY_REQUEST_RESPONSE_INFORMATION:
                case MQTT5_PROPERTY_MAXIMUM_QOS:
                case MQTT5_PROPERTY_RETAIN_AVAILABLE:
                case MQTT5_PROPERTY_WILDCARD_SUBSCRIPTION_AVAILABLE:
                case MQTT5_PROPERTY_SUBSCRIPTION_IDENTIFIER_AVAILABLE:
                case MQTT5_PROPERTY_SHARED_SUBSCRIPTION_AVAILABLE:
                    totalSize += 1U; /* Byte */
                    break;
                    
                case MQTT5_PROPERTY_SERVER_KEEP_ALIVE:
                case MQTT5_PROPERTY_RECEIVE_MAXIMUM:
                case MQTT5_PROPERTY_TOPIC_ALIAS_MAXIMUM:
                case MQTT5_PROPERTY_TOPIC_ALIAS:
                    totalSize += 2U; /* Two Byte Integer */
                    break;
                    
                case MQTT5_PROPERTY_MESSAGE_EXPIRY_INTERVAL:
                case MQTT5_PROPERTY_SESSION_EXPIRY_INTERVAL:
                case MQTT5_PROPERTY_WILL_DELAY_INTERVAL:
                case MQTT5_PROPERTY_MAXIMUM_PACKET_SIZE:
                    totalSize += 4U; /* Four Byte Integer */
                    break;
                    
                case MQTT5_PROPERTY_CONTENT_TYPE:
                case MQTT5_PROPERTY_RESPONSE_TOPIC:
                case MQTT5_PROPERTY_ASSIGNED_CLIENT_IDENTIFIER:
                case MQTT5_PROPERTY_AUTHENTICATION_METHOD:
                case MQTT5_PROPERTY_RESPONSE_INFORMATION:
                case MQTT5_PROPERTY_SERVER_REFERENCE:
                case MQTT5_PROPERTY_REASON_STRING:
                    /* UTF-8 String: 2 bytes length + string */
                    totalSize += 2U + pProp->value.utf8String.length;
                    break;
                    
                case MQTT5_PROPERTY_CORRELATION_DATA:
                case MQTT5_PROPERTY_AUTHENTICATION_DATA:
                    /* Binary Data: 2 bytes length + data */
                    totalSize += 2U + pProp->value.binaryData.length;
                    break;
                    
                case MQTT5_PROPERTY_USER_PROPERTY:
                    /* User Property: key length + key + value length + value */
                    totalSize += 2U + pProp->value.userProperty.keyLength +
                                 2U + pProp->value.userProperty.valueLength;
                    break;
                    
                case MQTT5_PROPERTY_SUBSCRIPTION_IDENTIFIER:
                    /* Variable Byte Integer - simplified to 1-4 bytes */
                    totalSize += 4U; /* Conservative estimate */
                    break;
                    
                default:
                    /* Unknown property type */
                    break;
            }
        }
    }

    return totalSize;
}

/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

/**
 * @brief Encode a Variable Byte Integer
 */
static size_t encodeVariableByteInteger( uint8_t * pBuffer,
                                         uint32_t value )
{
    size_t bytesWritten = 0U;
    uint8_t encodedByte;

    do
    {
        encodedByte = ( uint8_t ) ( value % 128U );
        value = value / 128U;

        if( value > 0U )
        {
            encodedByte = encodedByte | 0x80U;
        }

        if( pBuffer != NULL )
        {
            pBuffer[ bytesWritten ] = encodedByte;
        }

        bytesWritten++;
    } while( value > 0U );

    return bytesWritten;
}

/*-----------------------------------------------------------*/

/**
 * @brief Decode a Variable Byte Integer
 */
size_t MQTT5_DecodeVariableByteInteger( const uint8_t * pBuffer,
                                        size_t bufferSize,
                                        uint32_t * pValue )
{
    size_t bytesRead = 0U;
    uint32_t multiplier = 1U;
    uint8_t encodedByte;

    *pValue = 0U;

    do
    {
        if( bytesRead >= bufferSize )
        {
            return 0U; /* Error: buffer too small */
        }

        encodedByte = pBuffer[ bytesRead ];
        *pValue += ( encodedByte & 0x7FU ) * multiplier;
        multiplier *= 128U;
        bytesRead++;

        if( bytesRead > 4U )
        {
            return 0U; /* Error: malformed VBI */
        }
    } while( ( encodedByte & 0x80U ) != 0U );

    return bytesRead;
}

/*-----------------------------------------------------------*/

MQTTStatus_t MQTT5_SerializeProperties( const MQTT5Properties_t * pProperties,
                                        uint8_t * pBuffer,
                                        size_t * pSize )
{
    MQTTStatus_t status = MQTTSuccess;
    size_t index = 0U;
    size_t i;
    size_t propertiesLength;
    size_t vbiLength;

    if( ( pProperties == NULL ) || ( pBuffer == NULL ) || ( pSize == NULL ) )
    {
        status = MQTTBadParameter;
    }
    else
    {
        /* Calculate properties length (excluding the length field itself) */
        propertiesLength = MQTT5_GetPropertiesSize( pProperties );

        /* Encode properties length as Variable Byte Integer */
        vbiLength = encodeVariableByteInteger( &pBuffer[ index ], ( uint32_t ) propertiesLength );
        index += vbiLength;

        /* Serialize each property */
        for( i = 0U; ( i < pProperties->count ) && ( status == MQTTSuccess ); i++ )
        {
            const MQTT5Property_t * pProp = &pProperties->pProperties[ i ];

            /* Write property identifier */
            pBuffer[ index ] = ( uint8_t ) pProp->type;
            index++;

            /* Write property value based on type */
            switch( pProp->type )
            {
                case MQTT5_PROPERTY_PAYLOAD_FORMAT_INDICATOR:
                case MQTT5_PROPERTY_REQUEST_PROBLEM_INFORMATION:
                case MQTT5_PROPERTY_REQUEST_RESPONSE_INFORMATION:
                case MQTT5_PROPERTY_MAXIMUM_QOS:
                case MQTT5_PROPERTY_RETAIN_AVAILABLE:
                case MQTT5_PROPERTY_WILDCARD_SUBSCRIPTION_AVAILABLE:
                case MQTT5_PROPERTY_SUBSCRIPTION_IDENTIFIER_AVAILABLE:
                case MQTT5_PROPERTY_SHARED_SUBSCRIPTION_AVAILABLE:
                    pBuffer[ index ] = pProp->value.byte;
                    index++;
                    break;

                case MQTT5_PROPERTY_SERVER_KEEP_ALIVE:
                case MQTT5_PROPERTY_RECEIVE_MAXIMUM:
                case MQTT5_PROPERTY_TOPIC_ALIAS_MAXIMUM:
                case MQTT5_PROPERTY_TOPIC_ALIAS:
                    pBuffer[ index ] = ( uint8_t ) ( pProp->value.twoByteInteger >> 8U );
                    pBuffer[ index + 1U ] = ( uint8_t ) ( pProp->value.twoByteInteger & 0xFFU );
                    index += 2U;
                    break;

                case MQTT5_PROPERTY_MESSAGE_EXPIRY_INTERVAL:
                case MQTT5_PROPERTY_SESSION_EXPIRY_INTERVAL:
                case MQTT5_PROPERTY_WILL_DELAY_INTERVAL:
                case MQTT5_PROPERTY_MAXIMUM_PACKET_SIZE:
                    pBuffer[ index ] = ( uint8_t ) ( pProp->value.fourByteInteger >> 24U );
                    pBuffer[ index + 1U ] = ( uint8_t ) ( ( pProp->value.fourByteInteger >> 16U ) & 0xFFU );
                    pBuffer[ index + 2U ] = ( uint8_t ) ( ( pProp->value.fourByteInteger >> 8U ) & 0xFFU );
                    pBuffer[ index + 3U ] = ( uint8_t ) ( pProp->value.fourByteInteger & 0xFFU );
                    index += 4U;
                    break;

                case MQTT5_PROPERTY_CONTENT_TYPE:
                case MQTT5_PROPERTY_RESPONSE_TOPIC:
                case MQTT5_PROPERTY_ASSIGNED_CLIENT_IDENTIFIER:
                case MQTT5_PROPERTY_AUTHENTICATION_METHOD:
                case MQTT5_PROPERTY_RESPONSE_INFORMATION:
                case MQTT5_PROPERTY_SERVER_REFERENCE:
                case MQTT5_PROPERTY_REASON_STRING:
                    /* UTF-8 String: length (2 bytes) + string */
                    pBuffer[ index ] = ( uint8_t ) ( pProp->value.utf8String.length >> 8U );
                    pBuffer[ index + 1U ] = ( uint8_t ) ( pProp->value.utf8String.length & 0xFFU );
                    index += 2U;
                    ( void ) memcpy( &pBuffer[ index ], pProp->value.utf8String.pString,
                                     pProp->value.utf8String.length );
                    index += pProp->value.utf8String.length;
                    break;

                case MQTT5_PROPERTY_CORRELATION_DATA:
                case MQTT5_PROPERTY_AUTHENTICATION_DATA:
                    /* Binary Data: length (2 bytes) + data */
                    pBuffer[ index ] = ( uint8_t ) ( pProp->value.binaryData.length >> 8U );
                    pBuffer[ index + 1U ] = ( uint8_t ) ( pProp->value.binaryData.length & 0xFFU );
                    index += 2U;
                    ( void ) memcpy( &pBuffer[ index ], pProp->value.binaryData.pData,
                                     pProp->value.binaryData.length );
                    index += pProp->value.binaryData.length;
                    break;

                case MQTT5_PROPERTY_USER_PROPERTY:
                    /* Key: length + string */
                    pBuffer[ index ] = ( uint8_t ) ( pProp->value.userProperty.keyLength >> 8U );
                    pBuffer[ index + 1U ] = ( uint8_t ) ( pProp->value.userProperty.keyLength & 0xFFU );
                    index += 2U;
                    ( void ) memcpy( &pBuffer[ index ], pProp->value.userProperty.pKey,
                                     pProp->value.userProperty.keyLength );
                    index += pProp->value.userProperty.keyLength;
                    /* Value: length + string */
                    pBuffer[ index ] = ( uint8_t ) ( pProp->value.userProperty.valueLength >> 8U );
                    pBuffer[ index + 1U ] = ( uint8_t ) ( pProp->value.userProperty.valueLength & 0xFFU );
                    index += 2U;
                    ( void ) memcpy( &pBuffer[ index ], pProp->value.userProperty.pValue,
                                     pProp->value.userProperty.valueLength );
                    index += pProp->value.userProperty.valueLength;
                    break;

                case MQTT5_PROPERTY_SUBSCRIPTION_IDENTIFIER:
                    /* Variable Byte Integer */
                    index += encodeVariableByteInteger( &pBuffer[ index ], pProp->value.fourByteInteger );
                    break;

                default:
                    status = MQTTBadParameter;
                    break;
            }
        }

        *pSize = index;
    }

    return status;
}

/*-----------------------------------------------------------*/

MQTTStatus_t MQTT5_DeserializeProperties( MQTT5Properties_t * pProperties,
                                          const uint8_t * pBuffer,
                                          size_t size )
{
    MQTTStatus_t status = MQTTSuccess;
    size_t index = 0U;
    uint32_t propertiesLength;
    size_t vbiLength;
    MQTT5Property_t property;

    if( ( pProperties == NULL ) || ( pBuffer == NULL ) )
    {
        status = MQTTBadParameter;
    }
    else
    {
        /* Decode properties length */
        vbiLength = MQTT5_DecodeVariableByteInteger( pBuffer, size, &propertiesLength );

        if( vbiLength == 0U )
        {
            status = MQTTBadParameter;
        }
        else
        {
            index = vbiLength;

            /* Parse properties */
            while( ( index < size ) && ( index < ( vbiLength + propertiesLength ) ) &&
                   ( status == MQTTSuccess ) )
            {
                /* Read property identifier */
                property.type = ( MQTT5PropertyType_t ) pBuffer[ index ];
                index++;

                /* Read property value based on type */
                switch( property.type )
                {
                    case MQTT5_PROPERTY_PAYLOAD_FORMAT_INDICATOR:
                    case MQTT5_PROPERTY_REQUEST_PROBLEM_INFORMATION:
                    case MQTT5_PROPERTY_REQUEST_RESPONSE_INFORMATION:
                    case MQTT5_PROPERTY_MAXIMUM_QOS:
                    case MQTT5_PROPERTY_RETAIN_AVAILABLE:
                    case MQTT5_PROPERTY_WILDCARD_SUBSCRIPTION_AVAILABLE:
                    case MQTT5_PROPERTY_SUBSCRIPTION_IDENTIFIER_AVAILABLE:
                    case MQTT5_PROPERTY_SHARED_SUBSCRIPTION_AVAILABLE:
                        property.value.byte = pBuffer[ index ];
                        index++;
                        break;

                    case MQTT5_PROPERTY_SERVER_KEEP_ALIVE:
                    case MQTT5_PROPERTY_RECEIVE_MAXIMUM:
                    case MQTT5_PROPERTY_TOPIC_ALIAS_MAXIMUM:
                    case MQTT5_PROPERTY_TOPIC_ALIAS:
                        property.value.twoByteInteger = ( ( uint16_t ) pBuffer[ index ] << 8U ) |
                                                        ( uint16_t ) pBuffer[ index + 1U ];
                        index += 2U;
                        break;

                    case MQTT5_PROPERTY_MESSAGE_EXPIRY_INTERVAL:
                    case MQTT5_PROPERTY_SESSION_EXPIRY_INTERVAL:
                    case MQTT5_PROPERTY_WILL_DELAY_INTERVAL:
                    case MQTT5_PROPERTY_MAXIMUM_PACKET_SIZE:
                        property.value.fourByteInteger = ( ( uint32_t ) pBuffer[ index ] << 24U ) |
                                                         ( ( uint32_t ) pBuffer[ index + 1U ] << 16U ) |
                                                         ( ( uint32_t ) pBuffer[ index + 2U ] << 8U ) |
                                                         ( uint32_t ) pBuffer[ index + 3U ];
                        index += 4U;
                        break;

                    case MQTT5_PROPERTY_CONTENT_TYPE:
                    case MQTT5_PROPERTY_RESPONSE_TOPIC:
                    case MQTT5_PROPERTY_ASSIGNED_CLIENT_IDENTIFIER:
                    case MQTT5_PROPERTY_AUTHENTICATION_METHOD:
                    case MQTT5_PROPERTY_RESPONSE_INFORMATION:
                    case MQTT5_PROPERTY_SERVER_REFERENCE:
                    case MQTT5_PROPERTY_REASON_STRING:
                        /* UTF-8 String */
                        property.value.utf8String.length = ( ( uint16_t ) pBuffer[ index ] << 8U ) |
                                                           ( uint16_t ) pBuffer[ index + 1U ];
                        index += 2U;
                        property.value.utf8String.pString = ( const char * ) &pBuffer[ index ];
                        index += property.value.utf8String.length;
                        break;

                    case MQTT5_PROPERTY_CORRELATION_DATA:
                    case MQTT5_PROPERTY_AUTHENTICATION_DATA:
                        /* Binary Data */
                        property.value.binaryData.length = ( ( uint16_t ) pBuffer[ index ] << 8U ) |
                                                           ( uint16_t ) pBuffer[ index + 1U ];
                        index += 2U;
                        property.value.binaryData.pData = &pBuffer[ index ];
                        index += property.value.binaryData.length;
                        break;

                    case MQTT5_PROPERTY_USER_PROPERTY:
                        /* Key */
                        property.value.userProperty.keyLength = ( ( uint16_t ) pBuffer[ index ] << 8U ) |
                                                                ( uint16_t ) pBuffer[ index + 1U ];
                        index += 2U;
                        property.value.userProperty.pKey = ( const char * ) &pBuffer[ index ];
                        index += property.value.userProperty.keyLength;
                        /* Value */
                        property.value.userProperty.valueLength = ( ( uint16_t ) pBuffer[ index ] << 8U ) |
                                                                  ( uint16_t ) pBuffer[ index + 1U ];
                        index += 2U;
                        property.value.userProperty.pValue = ( const char * ) &pBuffer[ index ];
                        index += property.value.userProperty.valueLength;
                        break;

                    case MQTT5_PROPERTY_SUBSCRIPTION_IDENTIFIER:
                        vbiLength = MQTT5_DecodeVariableByteInteger( &pBuffer[ index ], size - index,
                                                              &property.value.fourByteInteger );
                        if( vbiLength == 0U )
                        {
                            status = MQTTBadParameter;
                        }
                        else
                        {
                            index += vbiLength;
                        }
                        break;

                    default:
                        /* Unknown property - skip it */
                        status = MQTTBadParameter;
                        break;
                }

                /* Add property to collection */
                if( status == MQTTSuccess )
                {
                    status = MQTT5_AddProperty( pProperties, &property );
                }
            }
        }
    }

    return status;
}
