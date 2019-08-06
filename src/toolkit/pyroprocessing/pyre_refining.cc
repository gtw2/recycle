#include "pyre_refining.h"
#include "boost/bind.hpp"
#include <boost/math/tools/roots.hpp>


using cyclus::Material;
using cyclus::Composition;
using cyclus::CompMap;
using cyclus::ValueError;

using boost::math::tools::bisect;

namespace pyro {

Refine::Refine() {
  temp(0);
  pressure(0);
  rotation(0);
  b_size(0);
  Rtime(0);
}

Refine::Refine(double new_temp = 900, 
               double new_press = 760, 
               double new_rotation = 0, 
               double new_batch_size = 20,
               double new_rtime = 1
            ) 
            {
  SetCoeff();
  temp(new_temp);
  pressure(new_press);
  rotation(new_rotation);
  b_size(new_batch_size);
  Rtime(new_rtime);
}

void Refine::SetCoeff() {
  t0_ = 4.7369E-9;
  t1_ = -1.08337E-5;
  t2_ = 0.008069;
  t3_ = -0.9726;
  p0_ = -7.17631E-10;
  p1_ = 4.04545E-07;
  p2_ = -8.06336E-05;
  p3_ = 1.002;
  a0_ = 0.032;
  a1_ = 0.72;
  a2_ = 0.0338396;
  a3_ = 0.83667;
}

struct Refine::TerminationCondition {
    bool operator() (double min, double max)  {
        return abs(min - max) <= 0.000001;
    }
};

void Refine::DivertMat(std::string type, std::pair<std::string, std::string> location,
  double siphon) {
    if (type == "operator") {
      OpDivertMat(location, siphon);
    } 
}

void Refine::OpDivertMat(std::pair<std::string, std::string> location, double siphon) {
  std::string param = location.second;
  double new_val;

  double paramVal = subcomponents[param].back();
  double lower_bound = paramVal-(paramVal*0.1);
  double upper_bound = paramVal+(paramVal*0.1);
  if (param == "temp"){
    double new_eff = Thermal() * (1+siphon);
    ThermalFunc = boost::bind(&pyro::Refine::Thermal,this,_1,new_eff);
    new_val = Bisector(ThermalFunc, lower_bound, upper_bound);
  } else if (param == "pressure") {
  } else if (param == "rotation") {
  } else if (param == "batch") {
  } else {
  }
  subcomponents[param].push_back(new_val);
}

double Refine::Bisector(boost::function<double(double)> SolveFunc, double lower, double upper) {
  std::pair<double, double> result = bisect(ThermalFunc, 
      temp()-100.0, temp()+100.0, TerminationCondition());
  return (result.first + result.second) / 2;
}

double Refine::Efficiency() {
  return Thermal()*PressureEff()*Agitation();
}

double Refine::Thermal() {
  return Thermal(temp(),0);
}

double Refine::Thermal(double tmp,
                       double new_eff
) {
  return t0()*pow(tmp,3) + t1()*pow(tmp,2) + t2()*tmp + t3() - new_eff;
}

double Refine::PressureEff() {
  return PressureEff(pressure(),0);
}

double Refine::PressureEff(double prs,
                           double new_eff
) {
  return p0()*pow(prs, 3) + p1()*pow(prs, 2) + p2()*prs + p3() - new_eff;
}

double Refine::Agitation() {
  return Agitation(rotation(),0);
}

double Refine::Agitation(double rot,
                         double new_eff
) {
  double agi;
  if (rot <= 1) {
    agi = a0()*rot + a1() - new_eff;
  } else {
    agi = a2()*log(rot) + a3() - new_eff;
    if (agi > 1) {
      throw ValueError("Rotation efficiency cannot exceed 1");
    }
  }
  return agi;
}

double Refine::Throughput() {
  return b_size() / Rtime();
}

double Refine::t0() {
  return t0_;
}

double Refine::t1() {
  return t1_;
}

double Refine::t2() {
  return t2_;
}

double Refine::t3() {
  return t3_;
}

double Refine::p0() {
  return p0_;
}

double Refine::p1() {
  return p1_;
}

double Refine::p2() {
  return p2_;
}

double Refine::p3() {
  return p3_;
}

double Refine::a0() {
  return a0_;
}

double Refine::a1() {
  return a1_;
}

double Refine::a2() {
  return a2_;
}

double Refine::a3() {
  return a3_;
};
} // namespace pyro
