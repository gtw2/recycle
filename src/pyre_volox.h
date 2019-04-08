#ifndef RECYCLE_SRC_PYRE_VOLOX_H_
#define RECYCLE_SRC_PYRE_VOLOX_H_

#include "process.h"
#include "pyre.h"
#include "cyclus.h"
#include "recycle_version.h"

namespace recycle {

class Volox : public recycle::Process {

public:

Volox();

Volox(double volox_temp, double volox_time, 
	double volox_flowrate, double volox_volume);

/// @param feed feed snf
/// @param stream the separation efficiency of voloxidation
/// @return composition composition of the resulting product and waste
cyclus::Material::Ptr VoloxSepMaterial(std::map<int, double> effs,
	cyclus::Material::Ptr mat);

private:

/// @param temp temperature of the volox process
/// @param time time spent in the process
/// @param flow mass flow rate
/// @return efficiency separation efficiency of the voloxidation process
double Efficiency(std::vector<double> temp, std::vector<double> reprocess_time, 
	std::vector<double> flowrate);

/// @return throughput material throughput of voloxidation
double Throughput(std::vector<double> flowrate, 
	std::vector<double> reprocess_time, double volume);

};
}
#endif // RECYCLE_SRC_PYRE_VOLOX_H_
