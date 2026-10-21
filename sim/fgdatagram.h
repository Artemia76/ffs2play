/****************************************************************************
**
** Copyright (C) 2018 FSFranceSimulateur team.
** Contact: https://github.com/ffs2/ffs2play
**
** FFS2Play is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** FFS2Play is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** The license is as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this software. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
****************************************************************************/

/****************************************************************************
** fgdatagram.h is part of FF2Play project
**
** This is coder/decoder class for FlightGear Datagram
****************************************************************************/

#ifndef FGDATAGRAM_H
#define FGDATAGRAM_H

#include <QVector>
#include <QDataStream>

#include "tools/calculation.h"
#include "sim/aircraftstate.h"
#include "tools/clogger.h"

#define MAX_TEXT_SIZE 768 // Increased for 2017.3 to allow for long Emesary messages.

///
/// Imported from SimGear of FlightGear Project
/// simgear/src/props/props.hxx
///
namespace simgear
{

#ifdef NONE
#pragma warn A sloppy coder has defined NONE as a macro!
#undef NONE
#endif

#ifdef ALIAS
#pragma warn A sloppy coder has defined ALIAS as a macro!
#undef ALIAS
#endif

#ifdef UNSPECIFIED
#pragma warn A sloppy coder has defined UNSPECIFIED as a macro!
#undef UNSPECIFIED
#endif

#ifdef BOOL
#pragma warn A sloppy coder has defined BOOL as a macro!
#undef BOOL
#endif

#ifdef INT
#pragma warn A sloppy coder has defined INT as a macro!
#undef INT
#endif

#ifdef LONG
#pragma warn A sloppy coder has defined LONG as a macro!
#undef LONG
#endif

#ifdef FLOAT
#pragma warn A sloppy coder has defined FLOAT as a macro!
#undef FLOAT
#endif

#ifdef DOUBLE
#pragma warn A sloppy coder has defined DOUBLE as a macro!
#undef DOUBLE
#endif

#ifdef STRING
#pragma warn A sloppy coder has defined STRING as a macro!
#undef STRING
#endif

/**
 * The possible types of an SGPropertyNode. Types that appear after
 * EXTENDED are not stored in the SGPropertyNode itself.
 */
enum Type {
    NONE = 0, /**< The node hasn't been assigned a value yet. */
    ALIAS, /**< The node "points" to another node. */
    BOOL,
    INT,
    LONG,
    FLOAT,
    DOUBLE,
    STRING,
    UNSPECIFIED,
    EXTENDED, /**< The node's value is not stored in the property;
               * the actual value and type is retrieved from an
               * SGRawValue node. This type is never returned by @see
               * SGPropertyNode::getType.
               */
    // Extended properties
    VEC3D,
    VEC4D
};

// magic value for messages
const uint32_t MSG_MAGIC = 0x46474653;  // "FGFS"
// protocoll version
const uint32_t PROTO_VER = 0x00010001;  // 1.1

const int BOOLARRAY_BLOCKSIZE = 40;

const int BOOLARRAY_BASE_1 = 11000;
const int BOOLARRAY_BASE_2 = BOOLARRAY_BASE_1 + BOOLARRAY_BLOCKSIZE;
const int BOOLARRAY_BASE_3 = BOOLARRAY_BASE_2 + BOOLARRAY_BLOCKSIZE;
// define range of bool values for receive.
const int BOOLARRAY_START_ID = BOOLARRAY_BASE_1;
const int BOOLARRAY_END_ID = BOOLARRAY_BASE_3;
// Transmission uses a buffer to build the value for each array block.
const int MAX_BOOL_BUFFERS = 3;

// starting with 2018.1 we will now append new properties for each version at the end of the list - as this provides much
// better backwards compatibility.
const int V2018_1_BASE = 11990;
const int EMESARYBRIDGETYPE_BASE = 12200;
const int EMESARYBRIDGE_BASE = 12000;
const int V2018_3_BASE = 13000;
const int FALLBACK_MODEL_ID = 13000;

enum TransmissionType {
    TT_ASIS = 0, // transmit as defined in the property. This is the default
    TT_BOOL = BOOL,
    TT_INT = INT,
    TT_FLOAT = FLOAT,
    TT_STRING = STRING,
    TT_SHORTINT = 0x100,
    TT_SHORT_FLOAT_NORM = 0x101, // -1 .. 1 encoded into a short int (16 bit)
    TT_SHORT_FLOAT_1 = 0x102, //range -3276.7 .. 3276.7  float encoded into a short int (16 bit)
    TT_SHORT_FLOAT_2 = 0x103, //range -327.67 .. 327.67  float encoded into a short int (16 bit)
    TT_SHORT_FLOAT_3 = 0x104, //range -32.767 .. 32.767  float encoded into a short int (16 bit)
    TT_SHORT_FLOAT_4 = 0x105, //range -3.2767 .. 3.2767  float encoded into a short int (16 bit)
    TT_BOOLARRAY,
    TT_CHAR,
    TT_NOSEND, // Do not send this property - probably the receive element for a custom encoded property
};
/*
 * Definitions for the version of the protocol to use to transmit the items defined in the IdPropertyList
 *
 * The MP2017(V2) protocol allows for much better packing of strings, new types that are transmitted in 4bytes by transmitting
 * with short int (sometimes scaled) for the values (a lot of the properties that are transmitted will pack nicely into 16bits).
 * The MP2017(V2) protocol also allows for properties to be transmitted automatically as a different type and the encode/decode will
 * take this into consideration.
 * The pad magic is used to force older clients to use verifyProperties and as the first property transmitted is short int encoded it
 * will cause the rest of the packet to be discarded. This is the section of the packet that contains the properties defined in the list
 * here - the basic motion properties remain compatible, so the older client will see just the model, not chat, not animations etc.
 * The lower 16 bits of the prop_id (version) are the version and the upper 16bits are for meta data.
 */
const int V1_1_PROP_ID = 1;
const int V1_1_2_PROP_ID = 2;
const int V2_PROP_ID_PROTOCOL = 0x10001;

const int V2_PAD_MAGIC = 0x1face002;

struct IdPropertyList{
    unsigned short id;
    const char* name;
    Type type;
    TransmissionType TransmitAs;
};
// A static map of protocol property id values to property paths,
// This should be extendable dynamically for every specific aircraft ...
// For now only that static list
static const IdPropertyList sIdPropertyList[] = {
    { 10,  "sim/multiplay/protocol-version",           INT,   TT_SHORTINT },
    { 100, "surface-positions/left-aileron-pos-norm",  FLOAT, TT_SHORT_FLOAT_NORM },
    { 101, "surface-positions/right-aileron-pos-norm", FLOAT, TT_SHORT_FLOAT_NORM },
    { 102, "surface-positions/elevator-pos-norm",      FLOAT, TT_SHORT_FLOAT_NORM },
    { 103, "surface-positions/rudder-pos-norm",        FLOAT, TT_SHORT_FLOAT_NORM },
    { 104, "surface-positions/flap-pos-norm",          FLOAT, TT_SHORT_FLOAT_NORM },
    { 105, "surface-positions/speedbrake-pos-norm",    FLOAT, TT_SHORT_FLOAT_NORM },
    { 106, "gear/tailhook/position-norm",              FLOAT, TT_SHORT_FLOAT_NORM },
    { 107, "gear/launchbar/position-norm",             FLOAT, TT_SHORT_FLOAT_NORM },
    //
    { 108, "gear/launchbar/state",                     STRING, TT_ASIS},
    { 109, "gear/launchbar/holdback-position-norm",    FLOAT, TT_SHORT_FLOAT_NORM },
    { 110, "canopy/position-norm",                     FLOAT, TT_SHORT_FLOAT_NORM },
    { 111, "surface-positions/wing-pos-norm",          FLOAT, TT_SHORT_FLOAT_NORM },
    { 112, "surface-positions/wing-fold-pos-norm",     FLOAT, TT_SHORT_FLOAT_NORM },

    // to enable decoding this is the transient ID record that is in the packet. This is not sent directly - instead it is the result
    // of the conversion of property 108.
    { 120, "gear/launchbar/state-value",               INT, TT_NOSEND },

    { 200, "gear/gear[0]/compression-norm",           FLOAT, TT_SHORT_FLOAT_NORM },
    { 201, "gear/gear[0]/position-norm",              FLOAT, TT_SHORT_FLOAT_NORM },
    { 210, "gear/gear[1]/compression-norm",           FLOAT, TT_SHORT_FLOAT_NORM },
    { 211, "gear/gear[1]/position-norm",              FLOAT, TT_SHORT_FLOAT_NORM },
    { 220, "gear/gear[2]/compression-norm",           FLOAT, TT_SHORT_FLOAT_NORM },
    { 221, "gear/gear[2]/position-norm",              FLOAT, TT_SHORT_FLOAT_NORM },
    { 230, "gear/gear[3]/compression-norm",           FLOAT, TT_SHORT_FLOAT_NORM },
    { 231, "gear/gear[3]/position-norm",              FLOAT, TT_SHORT_FLOAT_NORM },
    { 240, "gear/gear[4]/compression-norm",           FLOAT, TT_SHORT_FLOAT_NORM },
    { 241, "gear/gear[4]/position-norm",              FLOAT, TT_SHORT_FLOAT_NORM },

    { 300, "engines/engine[0]/n1",  FLOAT, TT_SHORT_FLOAT_1},
    { 301, "engines/engine[0]/n2",  FLOAT, TT_SHORT_FLOAT_1},
    { 302, "engines/engine[0]/rpm", FLOAT, TT_SHORT_FLOAT_1},
    { 310, "engines/engine[1]/n1",  FLOAT, TT_SHORT_FLOAT_1},
    { 311, "engines/engine[1]/n2",  FLOAT, TT_SHORT_FLOAT_1},
    { 312, "engines/engine[1]/rpm", FLOAT, TT_SHORT_FLOAT_1},
    { 320, "engines/engine[2]/n1",  FLOAT, TT_SHORT_FLOAT_1},
    { 321, "engines/engine[2]/n2",  FLOAT, TT_SHORT_FLOAT_1},
    { 322, "engines/engine[2]/rpm", FLOAT, TT_SHORT_FLOAT_1},
    { 330, "engines/engine[3]/n1",  FLOAT, TT_SHORT_FLOAT_1},
    { 331, "engines/engine[3]/n2",  FLOAT, TT_SHORT_FLOAT_1},
    { 332, "engines/engine[3]/rpm", FLOAT, TT_SHORT_FLOAT_1},
    { 340, "engines/engine[4]/n1",  FLOAT, TT_SHORT_FLOAT_1},
    { 341, "engines/engine[4]/n2",  FLOAT, TT_SHORT_FLOAT_1},
    { 342, "engines/engine[4]/rpm", FLOAT, TT_SHORT_FLOAT_1},
    { 350, "engines/engine[5]/n1",  FLOAT, TT_SHORT_FLOAT_1},
    { 351, "engines/engine[5]/n2",  FLOAT, TT_SHORT_FLOAT_1},
    { 352, "engines/engine[5]/rpm", FLOAT, TT_SHORT_FLOAT_1},
    { 360, "engines/engine[6]/n1",  FLOAT, TT_SHORT_FLOAT_1},
    { 361, "engines/engine[6]/n2",  FLOAT, TT_SHORT_FLOAT_1},
    { 362, "engines/engine[6]/rpm", FLOAT, TT_SHORT_FLOAT_1},
    { 370, "engines/engine[7]/n1",  FLOAT, TT_SHORT_FLOAT_1},
    { 371, "engines/engine[7]/n2",  FLOAT, TT_SHORT_FLOAT_1},
    { 372, "engines/engine[7]/rpm", FLOAT, TT_SHORT_FLOAT_1},
    { 380, "engines/engine[8]/n1",  FLOAT, TT_SHORT_FLOAT_1},
    { 381, "engines/engine[8]/n2",  FLOAT, TT_SHORT_FLOAT_1},
    { 382, "engines/engine[8]/rpm", FLOAT, TT_SHORT_FLOAT_1},
    { 390, "engines/engine[9]/n1",  FLOAT, TT_SHORT_FLOAT_1},
    { 391, "engines/engine[9]/n2",  FLOAT, TT_SHORT_FLOAT_1},
    { 392, "engines/engine[9]/rpm", FLOAT, TT_SHORT_FLOAT_1},

    { 800, "rotors/main/rpm", FLOAT, TT_SHORT_FLOAT_1},
    { 801, "rotors/tail/rpm", FLOAT, TT_SHORT_FLOAT_1},
    { 810, "rotors/main/blade[0]/position-deg",  FLOAT, TT_SHORT_FLOAT_3},
    { 811, "rotors/main/blade[1]/position-deg",  FLOAT, TT_SHORT_FLOAT_3},
    { 812, "rotors/main/blade[2]/position-deg",  FLOAT, TT_SHORT_FLOAT_3},
    { 813, "rotors/main/blade[3]/position-deg",  FLOAT, TT_SHORT_FLOAT_3},
    { 820, "rotors/main/blade[0]/flap-deg",  FLOAT, TT_SHORT_FLOAT_3},
    { 821, "rotors/main/blade[1]/flap-deg",  FLOAT, TT_SHORT_FLOAT_3},
    { 822, "rotors/main/blade[2]/flap-deg",  FLOAT, TT_SHORT_FLOAT_3},
    { 823, "rotors/main/blade[3]/flap-deg",  FLOAT, TT_SHORT_FLOAT_3},
    { 830, "rotors/tail/blade[0]/position-deg",  FLOAT, TT_SHORT_FLOAT_3},
    { 831, "rotors/tail/blade[1]/position-deg",  FLOAT, TT_SHORT_FLOAT_3},

    { 900, "sim/hitches/aerotow/tow/length",                       FLOAT, TT_ASIS},
    { 901, "sim/hitches/aerotow/tow/elastic-constant",             FLOAT, TT_ASIS},
    { 902, "sim/hitches/aerotow/tow/weight-per-m-kg-m",            FLOAT, TT_ASIS},
    { 903, "sim/hitches/aerotow/tow/dist",                         FLOAT, TT_ASIS},
    { 904, "sim/hitches/aerotow/tow/connected-to-property-node",   BOOL, TT_ASIS},
    { 905, "sim/hitches/aerotow/tow/connected-to-ai-or-mp-callsign",   STRING, TT_ASIS},
    { 906, "sim/hitches/aerotow/tow/brake-force",                  FLOAT, TT_ASIS},
    { 907, "sim/hitches/aerotow/tow/end-force-x",                  FLOAT, TT_ASIS},
    { 908, "sim/hitches/aerotow/tow/end-force-y",                  FLOAT, TT_ASIS},
    { 909, "sim/hitches/aerotow/tow/end-force-z",                  FLOAT, TT_ASIS},
    { 930, "sim/hitches/aerotow/is-slave",                         BOOL, TT_ASIS},
    { 931, "sim/hitches/aerotow/speed-in-tow-direction",           FLOAT, TT_ASIS},
    { 932, "sim/hitches/aerotow/open",                             BOOL, TT_ASIS},
    { 933, "sim/hitches/aerotow/local-pos-x",                      FLOAT, TT_ASIS},
    { 934, "sim/hitches/aerotow/local-pos-y",                      FLOAT, TT_ASIS},
    { 935, "sim/hitches/aerotow/local-pos-z",                      FLOAT, TT_ASIS},

    { 1001, "controls/flight/slats",  FLOAT, TT_SHORT_FLOAT_4},
    { 1002, "controls/flight/speedbrake",  FLOAT, TT_SHORT_FLOAT_4},
    { 1003, "controls/flight/spoilers",  FLOAT, TT_SHORT_FLOAT_4},
    { 1004, "controls/gear/gear-down",  FLOAT, TT_SHORT_FLOAT_4},
    { 1005, "controls/lighting/nav-lights",  FLOAT, TT_SHORT_FLOAT_3},
    { 1006, "controls/armament/station[0]/jettison-all",  BOOL, TT_SHORTINT},

    { 1100, "sim/model/variant", INT, TT_ASIS},
    { 1101, "sim/model/livery/file", STRING, TT_ASIS},

    { 1200, "environment/wildfire/data", STRING, TT_ASIS},
    { 1201, "environment/contrail", INT, TT_SHORTINT},

    { 1300, "tanker", INT, TT_SHORTINT},

    { 1400, "scenery/events", STRING, TT_ASIS},

    { 1500, "instrumentation/transponder/transmitted-id", INT, TT_SHORTINT},
    { 1501, "instrumentation/transponder/altitude", INT, TT_ASIS},
    { 1502, "instrumentation/transponder/ident", BOOL, TT_SHORTINT},
    { 1503, "instrumentation/transponder/inputs/mode", INT, TT_SHORTINT},
    { 1504, "instrumentation/transponder/ground-bit", BOOL, TT_SHORTINT},
    { 1505, "instrumentation/transponder/airspeed-kt", INT, TT_SHORTINT},

    { 10001, "sim/multiplay/transmission-freq-hz",  STRING, TT_ASIS},
    { 10002, "sim/multiplay/chat",  STRING, TT_ASIS},

    { 10100, "sim/multiplay/generic/string[0]", STRING, TT_ASIS},
    { 10101, "sim/multiplay/generic/string[1]", STRING, TT_ASIS},
    { 10102, "sim/multiplay/generic/string[2]", STRING, TT_ASIS},
    { 10103, "sim/multiplay/generic/string[3]", STRING, TT_ASIS},
    { 10104, "sim/multiplay/generic/string[4]", STRING, TT_ASIS},
    { 10105, "sim/multiplay/generic/string[5]", STRING, TT_ASIS},
    { 10106, "sim/multiplay/generic/string[6]", STRING, TT_ASIS},
    { 10107, "sim/multiplay/generic/string[7]", STRING, TT_ASIS},
    { 10108, "sim/multiplay/generic/string[8]", STRING, TT_ASIS},
    { 10109, "sim/multiplay/generic/string[9]", STRING, TT_ASIS},
    { 10110, "sim/multiplay/generic/string[10]", STRING, TT_ASIS},
    { 10111, "sim/multiplay/generic/string[11]", STRING, TT_ASIS},
    { 10112, "sim/multiplay/generic/string[12]", STRING, TT_ASIS},
    { 10113, "sim/multiplay/generic/string[13]", STRING, TT_ASIS},
    { 10114, "sim/multiplay/generic/string[14]", STRING, TT_ASIS},
    { 10115, "sim/multiplay/generic/string[15]", STRING, TT_ASIS},
    { 10116, "sim/multiplay/generic/string[16]", STRING, TT_ASIS},
    { 10117, "sim/multiplay/generic/string[17]", STRING, TT_ASIS},
    { 10118, "sim/multiplay/generic/string[18]", STRING, TT_ASIS},
    { 10119, "sim/multiplay/generic/string[19]", STRING, TT_ASIS},

    { 10200, "sim/multiplay/generic/float[0]", FLOAT, TT_ASIS},
    { 10201, "sim/multiplay/generic/float[1]", FLOAT, TT_ASIS},
    { 10202, "sim/multiplay/generic/float[2]", FLOAT, TT_ASIS},
    { 10203, "sim/multiplay/generic/float[3]", FLOAT, TT_ASIS},
    { 10204, "sim/multiplay/generic/float[4]", FLOAT, TT_ASIS},
    { 10205, "sim/multiplay/generic/float[5]", FLOAT, TT_ASIS},
    { 10206, "sim/multiplay/generic/float[6]", FLOAT, TT_ASIS},
    { 10207, "sim/multiplay/generic/float[7]", FLOAT, TT_ASIS},
    { 10208, "sim/multiplay/generic/float[8]", FLOAT, TT_ASIS},
    { 10209, "sim/multiplay/generic/float[9]", FLOAT, TT_ASIS},
    { 10210, "sim/multiplay/generic/float[10]", FLOAT, TT_ASIS},
    { 10211, "sim/multiplay/generic/float[11]", FLOAT, TT_ASIS},
    { 10212, "sim/multiplay/generic/float[12]", FLOAT, TT_ASIS},
    { 10213, "sim/multiplay/generic/float[13]", FLOAT, TT_ASIS},
    { 10214, "sim/multiplay/generic/float[14]", FLOAT, TT_ASIS},
    { 10215, "sim/multiplay/generic/float[15]", FLOAT, TT_ASIS},
    { 10216, "sim/multiplay/generic/float[16]", FLOAT, TT_ASIS},
    { 10217, "sim/multiplay/generic/float[17]", FLOAT, TT_ASIS},
    { 10218, "sim/multiplay/generic/float[18]", FLOAT, TT_ASIS},
    { 10219, "sim/multiplay/generic/float[19]", FLOAT, TT_ASIS},

    { 10220, "sim/multiplay/generic/float[20]", FLOAT, TT_ASIS},
    { 10221, "sim/multiplay/generic/float[21]", FLOAT, TT_ASIS},
    { 10222, "sim/multiplay/generic/float[22]", FLOAT, TT_ASIS},
    { 10223, "sim/multiplay/generic/float[23]", FLOAT, TT_ASIS},
    { 10224, "sim/multiplay/generic/float[24]", FLOAT, TT_ASIS},
    { 10225, "sim/multiplay/generic/float[25]", FLOAT, TT_ASIS},
    { 10226, "sim/multiplay/generic/float[26]", FLOAT, TT_ASIS},
    { 10227, "sim/multiplay/generic/float[27]", FLOAT, TT_ASIS},
    { 10228, "sim/multiplay/generic/float[28]", FLOAT, TT_ASIS},
    { 10229, "sim/multiplay/generic/float[29]", FLOAT, TT_ASIS},
    { 10230, "sim/multiplay/generic/float[30]", FLOAT, TT_ASIS},
    { 10231, "sim/multiplay/generic/float[31]", FLOAT, TT_ASIS},
    { 10232, "sim/multiplay/generic/float[32]", FLOAT, TT_ASIS},
    { 10233, "sim/multiplay/generic/float[33]", FLOAT, TT_ASIS},
    { 10234, "sim/multiplay/generic/float[34]", FLOAT, TT_ASIS},
    { 10235, "sim/multiplay/generic/float[35]", FLOAT, TT_ASIS},
    { 10236, "sim/multiplay/generic/float[36]", FLOAT, TT_ASIS},
    { 10237, "sim/multiplay/generic/float[37]", FLOAT, TT_ASIS},
    { 10238, "sim/multiplay/generic/float[38]", FLOAT, TT_ASIS},
    { 10239, "sim/multiplay/generic/float[39]", FLOAT, TT_ASIS},

    { 10300, "sim/multiplay/generic/int[0]", INT, TT_ASIS},
    { 10301, "sim/multiplay/generic/int[1]", INT, TT_ASIS},
    { 10302, "sim/multiplay/generic/int[2]", INT, TT_ASIS},
    { 10303, "sim/multiplay/generic/int[3]", INT, TT_ASIS},
    { 10304, "sim/multiplay/generic/int[4]", INT, TT_ASIS},
    { 10305, "sim/multiplay/generic/int[5]", INT, TT_ASIS},
    { 10306, "sim/multiplay/generic/int[6]", INT, TT_ASIS},
    { 10307, "sim/multiplay/generic/int[7]", INT, TT_ASIS},
    { 10308, "sim/multiplay/generic/int[8]", INT, TT_ASIS},
    { 10309, "sim/multiplay/generic/int[9]", INT, TT_ASIS},
    { 10310, "sim/multiplay/generic/int[10]", INT, TT_ASIS},
    { 10311, "sim/multiplay/generic/int[11]", INT, TT_ASIS},
    { 10312, "sim/multiplay/generic/int[12]", INT, TT_ASIS},
    { 10313, "sim/multiplay/generic/int[13]", INT, TT_ASIS},
    { 10314, "sim/multiplay/generic/int[14]", INT, TT_ASIS},
    { 10315, "sim/multiplay/generic/int[15]", INT, TT_ASIS},
    { 10316, "sim/multiplay/generic/int[16]", INT, TT_ASIS},
    { 10317, "sim/multiplay/generic/int[17]", INT, TT_ASIS},
    { 10318, "sim/multiplay/generic/int[18]", INT, TT_ASIS},
    { 10319, "sim/multiplay/generic/int[19]", INT, TT_ASIS},

    { 10500, "sim/multiplay/generic/short[0]", INT, TT_SHORTINT},
    { 10501, "sim/multiplay/generic/short[1]", INT, TT_SHORTINT},
    { 10502, "sim/multiplay/generic/short[2]", INT, TT_SHORTINT},
    { 10503, "sim/multiplay/generic/short[3]", INT, TT_SHORTINT},
    { 10504, "sim/multiplay/generic/short[4]", INT, TT_SHORTINT},
    { 10505, "sim/multiplay/generic/short[5]", INT, TT_SHORTINT},
    { 10506, "sim/multiplay/generic/short[6]", INT, TT_SHORTINT},
    { 10507, "sim/multiplay/generic/short[7]", INT, TT_SHORTINT},
    { 10508, "sim/multiplay/generic/short[8]", INT, TT_SHORTINT},
    { 10509, "sim/multiplay/generic/short[9]", INT, TT_SHORTINT},
    { 10510, "sim/multiplay/generic/short[10]", INT, TT_SHORTINT},
    { 10511, "sim/multiplay/generic/short[11]", INT, TT_SHORTINT},
    { 10512, "sim/multiplay/generic/short[12]", INT, TT_SHORTINT},
    { 10513, "sim/multiplay/generic/short[13]", INT, TT_SHORTINT},
    { 10514, "sim/multiplay/generic/short[14]", INT, TT_SHORTINT},
    { 10515, "sim/multiplay/generic/short[15]", INT, TT_SHORTINT},
    { 10516, "sim/multiplay/generic/short[16]", INT, TT_SHORTINT},
    { 10517, "sim/multiplay/generic/short[17]", INT, TT_SHORTINT},
    { 10518, "sim/multiplay/generic/short[18]", INT, TT_SHORTINT},
    { 10519, "sim/multiplay/generic/short[19]", INT, TT_SHORTINT},
    { 10520, "sim/multiplay/generic/short[20]", INT, TT_SHORTINT},
    { 10521, "sim/multiplay/generic/short[21]", INT, TT_SHORTINT},
    { 10522, "sim/multiplay/generic/short[22]", INT, TT_SHORTINT},
    { 10523, "sim/multiplay/generic/short[23]", INT, TT_SHORTINT},
    { 10524, "sim/multiplay/generic/short[24]", INT, TT_SHORTINT},
    { 10525, "sim/multiplay/generic/short[25]", INT, TT_SHORTINT},
    { 10526, "sim/multiplay/generic/short[26]", INT, TT_SHORTINT},
    { 10527, "sim/multiplay/generic/short[27]", INT, TT_SHORTINT},
    { 10528, "sim/multiplay/generic/short[28]", INT, TT_SHORTINT},
    { 10529, "sim/multiplay/generic/short[29]", INT, TT_SHORTINT},
    { 10530, "sim/multiplay/generic/short[30]", INT, TT_SHORTINT},
    { 10531, "sim/multiplay/generic/short[31]", INT, TT_SHORTINT},
    { 10532, "sim/multiplay/generic/short[32]", INT, TT_SHORTINT},
    { 10533, "sim/multiplay/generic/short[33]", INT, TT_SHORTINT},
    { 10534, "sim/multiplay/generic/short[34]", INT, TT_SHORTINT},
    { 10535, "sim/multiplay/generic/short[35]", INT, TT_SHORTINT},
    { 10536, "sim/multiplay/generic/short[36]", INT, TT_SHORTINT},
    { 10537, "sim/multiplay/generic/short[37]", INT, TT_SHORTINT},
    { 10538, "sim/multiplay/generic/short[38]", INT, TT_SHORTINT},
    { 10539, "sim/multiplay/generic/short[39]", INT, TT_SHORTINT},
    { 10540, "sim/multiplay/generic/short[40]", INT, TT_SHORTINT},
    { 10541, "sim/multiplay/generic/short[41]", INT, TT_SHORTINT},
    { 10542, "sim/multiplay/generic/short[42]", INT, TT_SHORTINT},
    { 10543, "sim/multiplay/generic/short[43]", INT, TT_SHORTINT},
    { 10544, "sim/multiplay/generic/short[44]", INT, TT_SHORTINT},
    { 10545, "sim/multiplay/generic/short[45]", INT, TT_SHORTINT},
    { 10546, "sim/multiplay/generic/short[46]", INT, TT_SHORTINT},
    { 10547, "sim/multiplay/generic/short[47]", INT, TT_SHORTINT},
    { 10548, "sim/multiplay/generic/short[48]", INT, TT_SHORTINT},
    { 10549, "sim/multiplay/generic/short[49]", INT, TT_SHORTINT},
    { 10550, "sim/multiplay/generic/short[50]", INT, TT_SHORTINT},
    { 10551, "sim/multiplay/generic/short[51]", INT, TT_SHORTINT},
    { 10552, "sim/multiplay/generic/short[52]", INT, TT_SHORTINT},
    { 10553, "sim/multiplay/generic/short[53]", INT, TT_SHORTINT},
    { 10554, "sim/multiplay/generic/short[54]", INT, TT_SHORTINT},
    { 10555, "sim/multiplay/generic/short[55]", INT, TT_SHORTINT},
    { 10556, "sim/multiplay/generic/short[56]", INT, TT_SHORTINT},
    { 10557, "sim/multiplay/generic/short[57]", INT, TT_SHORTINT},
    { 10558, "sim/multiplay/generic/short[58]", INT, TT_SHORTINT},
    { 10559, "sim/multiplay/generic/short[59]", INT, TT_SHORTINT},
    { 10560, "sim/multiplay/generic/short[60]", INT, TT_SHORTINT},
    { 10561, "sim/multiplay/generic/short[61]", INT, TT_SHORTINT},
    { 10562, "sim/multiplay/generic/short[62]", INT, TT_SHORTINT},
    { 10563, "sim/multiplay/generic/short[63]", INT, TT_SHORTINT},
    { 10564, "sim/multiplay/generic/short[64]", INT, TT_SHORTINT},
    { 10565, "sim/multiplay/generic/short[65]", INT, TT_SHORTINT},
    { 10566, "sim/multiplay/generic/short[66]", INT, TT_SHORTINT},
    { 10567, "sim/multiplay/generic/short[67]", INT, TT_SHORTINT},
    { 10568, "sim/multiplay/generic/short[68]", INT, TT_SHORTINT},
    { 10569, "sim/multiplay/generic/short[69]", INT, TT_SHORTINT},
    { 10570, "sim/multiplay/generic/short[70]", INT, TT_SHORTINT},
    { 10571, "sim/multiplay/generic/short[71]", INT, TT_SHORTINT},
    { 10572, "sim/multiplay/generic/short[72]", INT, TT_SHORTINT},
    { 10573, "sim/multiplay/generic/short[73]", INT, TT_SHORTINT},
    { 10574, "sim/multiplay/generic/short[74]", INT, TT_SHORTINT},
    { 10575, "sim/multiplay/generic/short[75]", INT, TT_SHORTINT},
    { 10576, "sim/multiplay/generic/short[76]", INT, TT_SHORTINT},
    { 10577, "sim/multiplay/generic/short[77]", INT, TT_SHORTINT},
    { 10578, "sim/multiplay/generic/short[78]", INT, TT_SHORTINT},
    { 10579, "sim/multiplay/generic/short[79]", INT, TT_SHORTINT},

    { BOOLARRAY_BASE_1 +  0, "sim/multiplay/generic/bool[0]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 +  1, "sim/multiplay/generic/bool[1]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 +  2, "sim/multiplay/generic/bool[2]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 +  3, "sim/multiplay/generic/bool[3]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 +  4, "sim/multiplay/generic/bool[4]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 +  5, "sim/multiplay/generic/bool[5]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 +  6, "sim/multiplay/generic/bool[6]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 +  7, "sim/multiplay/generic/bool[7]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 +  8, "sim/multiplay/generic/bool[8]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 +  9, "sim/multiplay/generic/bool[9]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 10, "sim/multiplay/generic/bool[10]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 11, "sim/multiplay/generic/bool[11]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 12, "sim/multiplay/generic/bool[12]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 13, "sim/multiplay/generic/bool[13]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 14, "sim/multiplay/generic/bool[14]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 15, "sim/multiplay/generic/bool[15]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 16, "sim/multiplay/generic/bool[16]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 17, "sim/multiplay/generic/bool[17]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 18, "sim/multiplay/generic/bool[18]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 19, "sim/multiplay/generic/bool[19]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 20, "sim/multiplay/generic/bool[20]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 21, "sim/multiplay/generic/bool[21]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 22, "sim/multiplay/generic/bool[22]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 23, "sim/multiplay/generic/bool[23]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 24, "sim/multiplay/generic/bool[24]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 25, "sim/multiplay/generic/bool[25]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 26, "sim/multiplay/generic/bool[26]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 27, "sim/multiplay/generic/bool[27]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 28, "sim/multiplay/generic/bool[28]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 29, "sim/multiplay/generic/bool[29]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_1 + 30, "sim/multiplay/generic/bool[30]", BOOL, TT_BOOLARRAY},

    { BOOLARRAY_BASE_2 + 0, "sim/multiplay/generic/bool[31]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 1, "sim/multiplay/generic/bool[32]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 2, "sim/multiplay/generic/bool[33]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 3, "sim/multiplay/generic/bool[34]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 4, "sim/multiplay/generic/bool[35]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 5, "sim/multiplay/generic/bool[36]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 6, "sim/multiplay/generic/bool[37]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 7, "sim/multiplay/generic/bool[38]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 8, "sim/multiplay/generic/bool[39]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 9, "sim/multiplay/generic/bool[40]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 10, "sim/multiplay/generic/bool[41]", BOOL, TT_BOOLARRAY},
        // out of sequence between the block and the buffer becuase of a typo. repurpose the first as that way [72] will work
        // correctly on older versions.
    { BOOLARRAY_BASE_2 + 11, "sim/multiplay/generic/bool[91]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 12, "sim/multiplay/generic/bool[42]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 13, "sim/multiplay/generic/bool[43]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 14, "sim/multiplay/generic/bool[44]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 15, "sim/multiplay/generic/bool[45]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 16, "sim/multiplay/generic/bool[46]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 17, "sim/multiplay/generic/bool[47]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 18, "sim/multiplay/generic/bool[48]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 19, "sim/multiplay/generic/bool[49]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 20, "sim/multiplay/generic/bool[50]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 21, "sim/multiplay/generic/bool[51]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 22, "sim/multiplay/generic/bool[52]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 23, "sim/multiplay/generic/bool[53]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 24, "sim/multiplay/generic/bool[54]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 25, "sim/multiplay/generic/bool[55]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 26, "sim/multiplay/generic/bool[56]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 27, "sim/multiplay/generic/bool[57]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 28, "sim/multiplay/generic/bool[58]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 29, "sim/multiplay/generic/bool[59]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_2 + 30, "sim/multiplay/generic/bool[60]", BOOL, TT_BOOLARRAY},

    { BOOLARRAY_BASE_3 + 0, "sim/multiplay/generic/bool[61]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 1, "sim/multiplay/generic/bool[62]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 2, "sim/multiplay/generic/bool[63]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 3, "sim/multiplay/generic/bool[64]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 4, "sim/multiplay/generic/bool[65]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 5, "sim/multiplay/generic/bool[66]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 6, "sim/multiplay/generic/bool[67]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 7, "sim/multiplay/generic/bool[68]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 8, "sim/multiplay/generic/bool[69]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 9, "sim/multiplay/generic/bool[70]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 10, "sim/multiplay/generic/bool[71]", BOOL, TT_BOOLARRAY},
        // out of sequence between the block and the buffer becuase of a typo. repurpose the first as that way [72] will work
        // correctly on older versions.
    { BOOLARRAY_BASE_3 + 11, "sim/multiplay/generic/bool[92]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 12, "sim/multiplay/generic/bool[72]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 13, "sim/multiplay/generic/bool[73]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 14, "sim/multiplay/generic/bool[74]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 15, "sim/multiplay/generic/bool[75]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 16, "sim/multiplay/generic/bool[76]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 17, "sim/multiplay/generic/bool[77]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 18, "sim/multiplay/generic/bool[78]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 19, "sim/multiplay/generic/bool[79]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 20, "sim/multiplay/generic/bool[80]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 21, "sim/multiplay/generic/bool[81]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 22, "sim/multiplay/generic/bool[82]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 23, "sim/multiplay/generic/bool[83]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 24, "sim/multiplay/generic/bool[84]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 25, "sim/multiplay/generic/bool[85]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 26, "sim/multiplay/generic/bool[86]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 27, "sim/multiplay/generic/bool[87]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 28, "sim/multiplay/generic/bool[88]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 29, "sim/multiplay/generic/bool[89]", BOOL, TT_BOOLARRAY},
    { BOOLARRAY_BASE_3 + 30, "sim/multiplay/generic/bool[90]", BOOL, TT_BOOLARRAY},


    { V2018_1_BASE + 0,  "sim/multiplay/mp-clock-mode", INT, TT_SHORTINT},
        // Direct support for emesary bridge properties. This is mainly to ensure that these properties do not overlap with the string
        // properties; although the emesary bridge can use any string property.
    { EMESARYBRIDGE_BASE + 0,  "sim/multiplay/emesary/bridge[0]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 1,  "sim/multiplay/emesary/bridge[1]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 2,  "sim/multiplay/emesary/bridge[2]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 3,  "sim/multiplay/emesary/bridge[3]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 4,  "sim/multiplay/emesary/bridge[4]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 5,  "sim/multiplay/emesary/bridge[5]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 6,  "sim/multiplay/emesary/bridge[6]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 7,  "sim/multiplay/emesary/bridge[7]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 8,  "sim/multiplay/emesary/bridge[8]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 9,  "sim/multiplay/emesary/bridge[9]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 10, "sim/multiplay/emesary/bridge[10]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 11, "sim/multiplay/emesary/bridge[11]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 12, "sim/multiplay/emesary/bridge[12]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 13, "sim/multiplay/emesary/bridge[13]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 14, "sim/multiplay/emesary/bridge[14]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 15, "sim/multiplay/emesary/bridge[15]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 16, "sim/multiplay/emesary/bridge[16]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 17, "sim/multiplay/emesary/bridge[17]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 18, "sim/multiplay/emesary/bridge[18]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 19, "sim/multiplay/emesary/bridge[19]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 20, "sim/multiplay/emesary/bridge[20]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 21, "sim/multiplay/emesary/bridge[21]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 22, "sim/multiplay/emesary/bridge[22]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 23, "sim/multiplay/emesary/bridge[23]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 24, "sim/multiplay/emesary/bridge[24]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 25, "sim/multiplay/emesary/bridge[25]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 26, "sim/multiplay/emesary/bridge[26]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 27, "sim/multiplay/emesary/bridge[27]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 28, "sim/multiplay/emesary/bridge[28]", STRING, TT_ASIS},
    { EMESARYBRIDGE_BASE + 29, "sim/multiplay/emesary/bridge[29]", STRING, TT_ASIS},

        // To allow the bridge to identify itself and allow quick filtering based on type/ID.
    { EMESARYBRIDGETYPE_BASE + 0,  "sim/multiplay/emesary/bridge-type[0]",  INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 1,  "sim/multiplay/emesary/bridge-type[1]",  INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 2,  "sim/multiplay/emesary/bridge-type[2]",  INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 3,  "sim/multiplay/emesary/bridge-type[3]",  INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 4,  "sim/multiplay/emesary/bridge-type[4]",  INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 5,  "sim/multiplay/emesary/bridge-type[5]",  INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 6,  "sim/multiplay/emesary/bridge-type[6]",  INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 7,  "sim/multiplay/emesary/bridge-type[7]",  INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 8,  "sim/multiplay/emesary/bridge-type[8]",  INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 9,  "sim/multiplay/emesary/bridge-type[9]",  INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 10, "sim/multiplay/emesary/bridge-type[10]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 11, "sim/multiplay/emesary/bridge-type[11]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 12, "sim/multiplay/emesary/bridge-type[12]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 13, "sim/multiplay/emesary/bridge-type[13]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 14, "sim/multiplay/emesary/bridge-type[14]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 15, "sim/multiplay/emesary/bridge-type[15]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 16, "sim/multiplay/emesary/bridge-type[16]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 17, "sim/multiplay/emesary/bridge-type[17]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 18, "sim/multiplay/emesary/bridge-type[18]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 19, "sim/multiplay/emesary/bridge-type[19]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 20, "sim/multiplay/emesary/bridge-type[20]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 21, "sim/multiplay/emesary/bridge-type[21]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 22, "sim/multiplay/emesary/bridge-type[22]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 23, "sim/multiplay/emesary/bridge-type[23]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 24, "sim/multiplay/emesary/bridge-type[24]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 25, "sim/multiplay/emesary/bridge-type[25]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 26, "sim/multiplay/emesary/bridge-type[26]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 27, "sim/multiplay/emesary/bridge-type[27]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 28, "sim/multiplay/emesary/bridge-type[28]", INT, TT_SHORTINT},
    { EMESARYBRIDGETYPE_BASE + 29, "sim/multiplay/emesary/bridge-type[29]", INT, TT_SHORTINT},

    { FALLBACK_MODEL_ID, "sim/model/fallback-model-index", INT, TT_SHORTINT},
};

/*
 * For the 2017.x version 2 protocol the properties are sent in two partitions,
 * the first of these is a V1 protocol packet (which should be fine with all clients), and a V2 partition
 * which will contain the newly supported shortint and fixed string encoding schemes.
 * This is to possibly allow for easier V1/V2 conversion - as the packet can simply be truncated at the
 * first V2 property based on ID.
 */
const int MAX_PARTITIONS = 2;
const unsigned int numProperties = (sizeof(sIdPropertyList) / sizeof(sIdPropertyList[0]));

struct ComparePropertyId
  {
    bool operator()(const IdPropertyList& lhs,
                    const IdPropertyList& rhs)
    {
      return lhs.id < rhs.id;
    }
    bool operator()(const IdPropertyList& lhs,
                    unsigned id)
    {
      return lhs.id < id;
    }
    bool operator()(unsigned id,
                    const IdPropertyList& rhs)
    {
      return id < rhs.id;
    }
  };

struct FGHeader
{
    quint32 Magic;
    quint32 Version;
    quint32 MsgId;
    quint32 MsgLen;
    quint32 Requested_Visibility;
    quint32 Replyport;
    char    CallSign[8];
};

struct FGPos
{
    char    ModelName[96];
    double  Time;
    double  Lag;
    double  Position[3];
    float   Orientation[3];
    float   LinearVelocity[3];
    float   AngularVelocity[3];
    float   LinearAccel[3];
    float   AngularAccel[3];
};

struct FGPropertyData
{
    quint16 id = 0;
    Type type = UNSPECIFIED;
    union
    {
        int int_value;
        float float_value;
    };
    /// Value of STRING and UNSPECIFIED properties (owned, copyable)
    QByteArray string_value;
    FGPropertyData() : int_value(0) {}
};

class FGDatagram
{
public:
                            FGDatagram();
                            ~FGDatagram();
    void                    Read(QDataStream& pStream);
    void                    Write(QDataStream& pStream);
    bool                    IsSane();
    bool                    GetHeader(FGHeader& pHeader);
    bool                    SetHeader(const FGHeader& pHeader);
    bool                    GetPos(FGPos& pPos);
    bool                    SetPos(FGPos& pPos);
    bool                    GetAirData(CAircraftState& pData);
    bool                    SetAirData(const CAircraftState& pData);

private:
    FGHeader                m_Header;
    FGPos                   m_Pos;
    QVector<FGPropertyData> m_Props;
    bool                    m_Sane;
    CLogger&                m_Log = CLogger::instance();
    bool                    GetProperty(QDataStream& pStream, FGPropertyData& pPropData);
const   IdPropertyList*         FindProperty(unsigned short pID);

};
} //Namespace SimGear
#endif // FGDATAGRAM_H
