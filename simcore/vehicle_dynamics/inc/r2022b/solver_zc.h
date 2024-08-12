/* Copyright 2002-2021 The MathWorks, Inc. */
/**
 *
 * Types and define related to zero crossing signals and events
 *
 */

/* Leading _ in header guards violates MISRA standard rule 21.1 */
#ifndef solver_zc_hpp_
#  define solver_zc_hpp_

/* slZcSignalType ==============================================================
 *
 */
typedef enum {

  SL_ZCS_TYPE_CONT = 0,
  SL_ZCS_TYPE_DISC = 1,
  SL_ZCS_TYPE_HYBRID = 2

} slZcSignalType;

/* slZcEventType ===============================================================
 *
 *  zcEvents are bit packed as follows:
 *
 *   .-----.-----.-----.-----.-----.-----.-----.-----.
 *   |     |     | z2n | p2z | p2n | z2p | n2z | n2p |
 *   '-----'-----'-----'-----'-----'-----'-----'-----'
 *            ^  |<----------- eventTypes ---------->|
 *            |
 *            `--- needsSolverReset
 *
 */
typedef uint8_T slZcEventType;
#  define SL_ZCS_EVENT_NUL 0x00U
#  define SL_ZCS_EVENT_N2P 0x01U
#  define SL_ZCS_EVENT_N2Z 0x02U
#  define SL_ZCS_EVENT_Z2P 0x04U
#  define SL_ZCS_EVENT_P2N 0x08U
#  define SL_ZCS_EVENT_P2Z 0x10U
#  define SL_ZCS_EVENT_Z2N 0x20U

#  define SL_ZCS_EVENT_ALL_UP (SL_ZCS_EVENT_N2P | SL_ZCS_EVENT_N2Z | SL_ZCS_EVENT_Z2P)

#  define SL_ZCS_EVENT_ALL_DN (SL_ZCS_EVENT_P2N | SL_ZCS_EVENT_P2Z | SL_ZCS_EVENT_Z2N)

#  define SL_ZCS_EVENT_ALL (SL_ZCS_EVENT_ALL_UP | SL_ZCS_EVENT_ALL_DN)

#  define slZcSignalGetNeedsSolverReset(a) (((a) & 0x40) != 0x00)

#  define slZcSignalSetNeedsSolverReset(a, v) (a) = (v) ? ((a) | 0x40) : ((a) & 0xBF)

/* slZcSignalAttribType ========================================================
 *
 *  ZcSignal attributes are bit packed as follows:
 *
 *   .-----.-----.-----.-----.-----.-----.-----.-----.
 *   |     |     | z2n | p2z | p2n | z2p | n2z | n2p |
 *   '-----'-----'-----'-----'-----'-----'-----'-----'
 *      ^     ^  |<---- which events to look for --->|
 *      |     |
 *      |     `--- isDiscrete
 *      |
 *      `--------- needsEventNotification
 */
typedef uint8_T slZcSignalAttribType;

#  define slZcSignalGetEventDirections(a) ((a) & 0x3F)

#  define slZcSignalSetEventDirections(a, d) (a) = (((a) & 0xC0) | (d))

#  define slZcSignalGetIsDiscrete(a) (((a) & 0x40) != 0x00)

#  define slZcSignalSetIsDiscrete(a, v) (a) = (v) ? ((a) | 0x40) : ((a) & 0xBF)

#  define slZcSignalGetNeedsEventNotification(a) (((a) & 0x80) != 0x00)

#  define slZcSignalSetNeedsEventNotification(a, v) (a) = (v) ? ((a) | 0x80) : ((a) & 0x7F)

/* slZcSignalSignType ==========================================================
 *
 */
typedef uint8_T slZcSignalSignType;
#  define SL_ZCS_SIGN_ZERO 0x00U
#  define SL_ZCS_SIGN_POS 0x01U
#  define SL_ZCS_SIGN_NEG 0x02U
#  define SL_ZCS_SIGN_UNKNOWN 0x03U

#endif /* _solver_zc_hpp_ */

/* eof */
