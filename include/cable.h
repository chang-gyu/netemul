#ifndef __CABLE_H__
#define __CABLE_H__

#include "component.h"

/* Class Cable extends Componenet */
typedef struct {
	Component;

	/* Cable property */
#define MB	1000000
	uint64_t	bandwidth;	///< Bandwidth for each line (bytes).
	double		error_rate;	///< Packet error rate. e.g. Packet loss rate (percentage).
	double		jitter;		///< Deviation from periodicity of a presumed value.
	uint64_t	latency;	///< Latency (micro-seconds)
} Cable; 

Cable* cable_create(uint64_t bandwidth, double error_rate, double jitter, 
		uint64_t latency);

#endif /* __CABLE_H__ */
