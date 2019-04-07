#ifndef RECYCLE_SRC_PYRE_H_
#define RECYCLE_SRC_PYRE_H_

#include "diversion.h"
#include "cyclus.h"
#include "recycle_version.h"
#include "pyre_volox.h"
#include "pyre_reduction.h"
#include "pyre_refining.h"
#include "pyre_winning.h"

namespace recycle {

/// SepMaterial returns a material object that represents the composition and
/// quantity resulting from the separation of material from mat using the given
/// mass-based efficiencies.  Each key in effs represents a nuclide or element
/// (canonical PyNE form), and each value is the corresponding mass-based
/// separations efficiency for that nuclide or element.  Note that this returns
/// an untracked material that should only be used for its composition and qty
/// - not in any real inventories, etc.

/// Pyre processes feed material into multiple waste streams according to their
/// respective sub-process. Separation uses mass-based efficiencies.
///
/// Streams are predefined, but isotopic efficiencies and stream name
/// can be specified by the user. Streams are specified as groups of
/// component-efficiency pairs where 'component' means either a particular
/// element or a particular nuclide.  Each component's paired efficiency
/// represents the mass fraction of that component in the feed that is
/// separated into that stream.  The efficiencies of a particular component
/// across all streams must sum up to less than or equal to one.  If less than
/// one, the remainining material is sent to a waste inventory and
/// (potentially) traded away from there.
///
/// The facility receives material into a feed inventory that it processes with
/// a specified throughput each time step.  Each output stream has a
/// corresponding output inventory size/limit.  If the facility is unable to
/// reduce its stocks by trading and hits this limit for any of its output
/// streams, further processing/separations of feed material will halt until
/// room is again available in the output streams.
class Volox;
class Reduct;
class Refine;
class Winning;

class Pyre 
  : public cyclus::Facility,
    public cyclus::toolkit::Position {
#pragma cyclus note { \
  "niche": "pyre", \
  "doc": \
    "Pyre processes feed material into multiple waste streams according to their" \
    " respective sub-process. Separation uses mass-based efficiencies." \
    "\n\n" \
    "Streams are predefined, but isotopic efficiencies and stream name" \
    " can be specified by the user. Streams are specified as groups of" \
    " component-efficiency pairs where 'component' means either a particular" \
    " element or a particular nuclide.  Each component's paired efficiency" \
    " represents the mass fraction of that component in the feed that is" \
    " separated into that stream.  The efficiencies of a particular component" \
    " across all streams must sum up to less than or equal to one.  If less than" \
    " one, the remainining material is sent to a waste inventory and" \
    " (potentially) traded away from there." \
    "\n\n" \
    "The facility receives material into a feed inventory that it processes with" \
    " a specified throughput each time step.  Each output stream has a" \
    " corresponding output inventory size/limit.  If the facility is unable to" \
    " reduce its stocks by trading and hits this limit for any of its output" \
    " streams, further processing/separations of feed material will halt until" \
    " room is again available in the output streams." \
    "", \
}
 public:
  Pyre(cyclus::Context* ctx);
  virtual ~Pyre(){};

  virtual std::string version() { return RECYCLE_VERSION; }

  virtual void Tick();
  virtual void Tock();
  virtual void EnterNotify();

  typedef std::pair<double, std::map<int, double> > Stream;
  typedef std::map<std::string, Stream> StreamSet;

  cyclus::Material::Ptr Separate(std::string name, Stream stream, 
    cyclus::Material::Ptr feed);

  virtual void AcceptMatlTrades(const std::vector<std::pair<
      cyclus::Trade<cyclus::Material>, cyclus::Material::Ptr> >& responses);

  virtual std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
  GetMatlRequests();

  virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr> GetMatlBids(
      cyclus::CommodMap<cyclus::Material>::type& commod_requests);

  virtual void GetMatlTrades(
      const std::vector<cyclus::Trade<cyclus::Material> >& trades,
      std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                            cyclus::Material::Ptr> >& responses);

  virtual bool CheckDecommissionCondition();

  #pragma cyclus clone
  #pragma cyclus initfromcopy
  #pragma cyclus infiletodb
  #pragma cyclus initfromdb
  #pragma cyclus schema
  #pragma cyclus annotations
  #pragma cyclus snapshot
  // the following pragmas are ommitted and the functions are written
  // manually in order to handle the vector of resource buffers:
  //
  //     #pragma cyclus snapshotinv
  //     #pragma cyclus initinv

  virtual cyclus::Inventories SnapshotInv();
  virtual void InitInv(cyclus::Inventories& inv);

 private:
  Volox v;
  Reduct rd;
  Refine rf;
  Winning w;

  #pragma cyclus var { \
    "doc": "Ordered list of commodities on which to request feed material to " \
           "separate. Order only matters for matching up with feed commodity " \
           "preferences if specified.", \
    "uilabel": "Feed Commodity List", \
    "uitype": ["oneormore", "incommodity"], \
  }
  std::vector<std::string> feed_commods;

  #pragma cyclus var { \
    "default": [], \
    "uilabel": "Feed Commodity Preference List", \
    "doc": "Feed commodity request preferences for each of the given feed " \
           "commodities (same order)." \
           " If unspecified, default is to use 1.0 for all "\
           "preferences.",                                                     \
  }
  std::vector<double> feed_commod_prefs;

  #pragma cyclus var { \
    "doc": "Name for recipe to be used in feed requests." \
           " Empty string results in use of a dummy recipe.", \
    "uilabel": "Feed Commodity Recipe List", \
    "uitype": "inrecipe", \
    "default": "", \
  }
  std::string feed_recipe;

  #pragma cyclus var { \
  "default": 900, \
  "doc": "The temperature of the Voloxidation process", \
  "tooltip": "Voloxidation Temperature", \
  "units": "C", \
  "uitype": "range", \
  "range": [500,1000], \
  "uilabel": "Volox Temp", \
  }
  double volox_temp;

  #pragma cyclus var { \
  "doc": "Time spent in the Voloxidation process", \
  "tooltip": "Voloxidation process time", \
  "units": "hrs", \
  "uitype": "range", \
  "range": [1,4], \
  "uilabel": "Volox Time", \
  }
  double volox_time;

  #pragma cyclus var { \
  "doc": "Material flowrate through Voloxidation", \
  "tooltip": "Voloxidation Flowrate", \
  "units": "cms-1", \
  "uitype": "range", \
  "range": [0,4.5], \
  "uilabel": "Volox Flowrate", \
  }
  double volox_flowrate;

  #pragma cyclus var { \
  "doc": "Volume of the voloxidation chamber", \
  "tooltip": "Voloxidation Volume", \
  "units": "m3", \
  "uitype": "range", \
  "range": [1,10], \
  "uilabel": "Volox Volume", \
  }
  double volox_volume;

  #pragma cyclus var { \
  "doc": "Current in the Reduction process", \
  "tooltip": "Electroreduction Current", \
  "units": "A", \
  "uitype": "range", \
  "range": [1,10], \
  "uilabel": "Reduction Current", \
  }
  double reduct_current;

  #pragma cyclus var { \
  "doc": "Weight Percent of Lithium Oxide in Reduction process", \
  "tooltip": "Lithium Oxide Weight Percent", \
  "units": "wt%", \
  "uitype": "range", \
  "range": [1,3], \
  "uilabel": "Reduction Li2O wt%", \
  }
  double reduct_lithium_oxide;

  #pragma cyclus var { \
  "doc": "Volume of the Electroreduction Chamber", \
  "tooltip": "Electroreduction Volume", \
  "units": "m3", \
  "uitype": "range", \
  "range": [1,10], \
  "uilabel": "Reduction Volume", \
  }
  double reduct_volume;

  #pragma cyclus var { \
  "doc": "Time spent in the Reduction process", \
  "tooltip": "Reduction process time", \
  "units": "hrs", \
  "uitype": "range", \
  "range": [1,4], \
  "uilabel": "Reduct Time", \
  }
  double reduct_time;

  #pragma cyclus var { \
  "doc": "Temperature of the Electrorefining process", \
  "tooltip": "Electrorefining Temperature", \
  "units": "C", \
  "uitype": "range", \
  "range": [500,1000], \
  "uilabel": "Refining Temperature", \
  }
  double refine_temp;

  #pragma cyclus var { \
  "doc": "Pressure of the Electrorefining process", \
  "tooltip": "Electrorefining Pressure", \
  "units": "mTorr", \
  "uitype": "range", \
  "range": [100,760], \
  "uilabel": "Refining Pressure", \
  }
  double refine_press;

  #pragma cyclus var { \
  "doc": "Anode Rotation rate in the Electrorefiner", \
  "tooltip": "Rotation speed", \
  "units": "rpm", \
  "uitype": "range", \
  "range": [0,100], \
  "uilabel": "Rotation speed", \
  }
  double refine_rotation;

  #pragma cyclus var { \
  "doc": "Batch Size of the Electrorefining process", \
  "tooltip": "Batch Size", \
  "units": "kg", \
  "uitype": "range", \
  "range": [10,40], \
  "uilabel": "Refiner Batch Size", \
  }
  double refine_batch_size;

  #pragma cyclus var { \
  "doc": "Time spent in the Refining process", \
  "tooltip": "Refining process time", \
  "units": "hrs", \
  "uitype": "range", \
  "range": [1,4], \
  "uilabel": "Refine Time", \
  }
  double refine_time;

  #pragma cyclus var { \
  "doc": "Current in the Electrowinning process", \
  "tooltip": "Electrowinning Current", \
  "units": "A", \
  "uitype": "range", \
  "range": [1,10], \
  "uilabel": "Electrowinning Current", \
  }
  double winning_current;

  #pragma cyclus var { \
  "doc": "Time spent in the Electrowinning process", \
  "tooltip": "Electrowinning Time", \
  "units": "hr", \
  "uitype": "range", \
  "range": [1,4], \
  "uilabel": "Electrowinning Time", \
  }
  double winning_time;

  #pragma cyclus var { \
  "doc": "Flowrate through the Electrowinning process", \
  "tooltip": "Flowrate in Electrowinning", \
  "units": "cms-1", \
  "uitype": "range", \
  "range": [0,4.5], \
  "uilabel": "Electrowinning Flowrate", \
  }
  double winning_flowrate;

  #pragma cyclus var { \
  "doc": "Volume of the Electrowinning process", \
  "tooltip": "Electrowinning Volume", \
  "units": "m3", \
  "uitype": "range", \
  "range": [1,10], \
  "uilabel": "Electrowinning Volume", \
  }
  double winning_volume;

  #pragma cyclus var { \
    "doc" : "Maximum amount of feed material to keep on hand.", \
    "uilabel": "Maximum Feed Inventory",                     \
    "units" : "kg", \
  }
  double feedbuf_size;

  #pragma cyclus var { \
    "capacity" : "feedbuf_size", \
  }
  cyclus::toolkit::ResBuf<cyclus::Material> feed;

  #pragma cyclus var { \
    "doc" : "Maximum quantity of feed material that can be processed per time "\
            "step.", \
    "uilabel": "Maximum Separations Throughput", \
    "default": 1e299, \
    "uitype": "range", \
    "range": [0.0, 1e299], \
    "units": "kgday-1", \
  }
  double throughput;

  #pragma cyclus var { \
    "doc": "Commodity on which to trade the leftover separated material " \
           "stream. This MUST NOT be the same as any commodity used to define "\
           "the other separations streams.", \
    "uitype": "outcommodity", \
    "uilabel": "Leftover Commodity", \
    "default": "default-waste-stream", \
  }
  std::string leftover_commod;

  #pragma cyclus var { \
    "doc" : "Maximum amount of leftover separated material (not included in" \
            " any other stream) that can be stored." \
            " If full, the facility halts operation until space becomes " \
            "available.", \
    "uilabel": "Maximum Leftover Inventory", \
    "default": 1e299, \
    "uitype": "range", \
    "range": [0.0, 1e299], \
    "units": "kg", \
  }
  double leftoverbuf_size;

 #pragma cyclus var { \
    "capacity" : "leftoverbuf_size", \
  }
  cyclus::toolkit::ResBuf<cyclus::Material> leftover;

  #pragma cyclus var { \
    "alias": ["streams", "commod", ["info", "buf_size", ["efficiencies", "comp", "eff"]]], \
    "uitype": ["oneormore", "outcommodity", ["pair", "double", ["oneormore", "nuclide", "double"]]], \
    "uilabel": "Separations Streams and Efficiencies", \
    "doc": "Output streams for separations." \
           " Each stream must have a unique name identifying the commodity on "\
           " which its material is traded," \
           " a max buffer capacity in kg (neg values indicate infinite size)," \
           " and a set of component efficiencies." \
           " 'comp' is a component to be separated into the stream" \
           " (e.g. U, Pu, etc.) and 'eff' is the mass fraction of the" \
           " component that is separated from the feed into this output" \
           " stream. If any stream buffer is full, the facility halts" \
           " operation until space becomes available." \
           " The sum total of all component efficiencies across streams must" \
           " be less than or equal to 1" \
           " (e.g. sum of U efficiencies for all streams must be <= 1).", \
  }
  std::map<std::string, std::pair<double, std::map<int, double> > > streams_;

  // custom SnapshotInv and InitInv and EnterNotify are used to persist this
  // state var.
  std::map<std::string, cyclus::toolkit::ResBuf<cyclus::Material> > streambufs;

  #pragma cyclus var { \
    "default": 0.0, \
    "uilabel": "Geographical latitude in degrees as a double", \
    "doc": "Latitude of the agent's geographical position. The value should " \
           "be expressed in degrees as a double." \
  }
  double latitude;

  #pragma cyclus var { \
    "default": 0.0, \
    "uilabel": "Geographical longitude in degrees as a double", \
    "doc": "Longitude of the agent's geographical position. The value should " \
           "be expressed in degrees as a double." \
  }
  double longitude;

  cyclus::toolkit::Position coordinates;

  /// Records an agent's latitude and longitude to the output db
  void RecordPosition();

  void Record(std::string name, double val, std::string type);

  void RecordStreams();
};

}  // namespace recycle

#endif  // RECYCLE_SRC_PYRE_H_
