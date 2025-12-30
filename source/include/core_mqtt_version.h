/**
 * @file core_mqtt_version.h
 * @brief MQTT Protocol Version Configuration
 *
 * This file defines the MQTT protocol version to use.
 * Set MQTT_VERSION to either MQTT_VERSION_3_1_1 or MQTT_VERSION_5_0.
 */

#ifndef CORE_MQTT_VERSION_H
#define CORE_MQTT_VERSION_H

/**
 * @brief MQTT Protocol Version Identifiers
 */
#define MQTT_VERSION_3_1_1    ( 311 )
#define MQTT_VERSION_5_0      ( 500 )

/**
 * @brief MQTT Protocol Version Selection
 *
 * Define MQTT_VERSION to select the protocol version:
 * - MQTT_VERSION_3_1_1 (311) for MQTT 3.1.1
 * - MQTT_VERSION_5_0 (500) for MQTT 5.0
 *
 * This can be set via:
 * 1. CMake: -DMQTT_VERSION_5=ON (for MQTT 5.0)
 * 2. Compiler: -DMQTT_VERSION=500
 * 3. core_mqtt_config.h: #define MQTT_VERSION MQTT_VERSION_5_0
 *
 * Default: MQTT 3.1.1
 */
#ifndef MQTT_VERSION
    #define MQTT_VERSION    MQTT_VERSION_3_1_1
#endif

/* Validate MQTT_VERSION */
#if ( MQTT_VERSION != MQTT_VERSION_3_1_1 ) && ( MQTT_VERSION != MQTT_VERSION_5_0 )
    #error "MQTT_VERSION must be either MQTT_VERSION_3_1_1 (311) or MQTT_VERSION_5_0 (500)"
#endif

/**
 * @brief Helper macros for version checking
 */
#define MQTT_VERSION_IS_3_1_1()    ( MQTT_VERSION == MQTT_VERSION_3_1_1 )
#define MQTT_VERSION_IS_5_0()      ( MQTT_VERSION == MQTT_VERSION_5_0 )

#endif /* CORE_MQTT_VERSION_H */
