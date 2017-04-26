#ifndef __CABLE_H__
#define __CABLE_H__

#include "component.h"

/* Class Cable extends Componenet */
typedef struct {
	Component;

	/* Cable property */
#define MB	1000000
#define BIT	8
	uint64_t	bandwidth;	///< Bandwidth for each line (bps).
	uint64_t	output_closed;///< Total size.
	double		error_rate;	///< Packet error rate. e.g. Packet loss rate (percentage).
	double		drop_rate;
	uint64_t	jitter;		///< Deviation from periodicity of a presumed value.
	uint64_t	latency;	///< Latency (micro-seconds)
} Cable;

Cable* cable_create(uint64_t bandwidth, double error_rate, double drop_rate,
        double jitter, uint64_t latency);

#endif /* __CABLE_H__ */
