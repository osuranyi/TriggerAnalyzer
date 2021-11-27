// -*- C++ -*-
//
// Package:    FSQ/JetTriggerAnalyzer
// Class:      JetTriggerAnalyzer
// 
/**\class JetTriggerAnalyzer JetTriggerAnalyzer.cc FSQ/JetTriggerAnalyzer/plugins/JetTriggerAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Oliver Suranyi
//         Created:  Wed, 06 Dec 2017 17:12:48 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/Common/interface/TriggerResults.h"

#include "DataFormats/JetReco/interface/PFJet.h"
#include "DataFormats/JetReco/interface/PFJetCollection.h"

//#include "HLTrigger/HLTcore/interface/HLTPrescaleProvider.h"
//#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"

#include "DataFormats/PatCandidates/interface/Jet.h"

#include "TTree.h"

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<> and also remove the line from
// constructor "usesResource("TFileService");"
// This will improve performance in multithreaded jobs.

const static int Max = 5000;

class JetTriggerAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
   public:
      explicit JetTriggerAnalyzer(const edm::ParameterSet&);
      ~JetTriggerAnalyzer();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;

      // ----------member data ---------------------------

      //HLTPrescaleProvider hltPrescaleProvider_;
      std::string trgResultsProcess_;

      edm::EDGetTokenT<edm::TriggerResults> token_hlt;
      //#edm::EDGetTokenT<edm::TriggerResults> token_hlt_rerun;

      edm::EDGetTokenT<std::vector<pat::Jet>> token_jets;

      TTree* Events;

      int evt, run, lumi;
      bool firstEvent;
      int* trigflag;
      //#int* trigflag_rerun;
      //int* prscflag;

      int jetmult;
      float pt[Max];
      float eta[Max];
      float phi[Max];

//      int jetmult_ak8;
//      float pt_ak8[Max];
//      float eta_ak8[Max];
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
JetTriggerAnalyzer::JetTriggerAnalyzer(const edm::ParameterSet& iConfig){ //: hltPrescaleProvider_(iConfig, consumesCollector(), *this){
  //now do what ever initialization is needed
  usesResource("TFileService");

  std::cout << "Constructor begin" << std::endl;

  token_hlt = consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("hltresults"));
  //#token_hlt_rerun = consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("hltresults_rerun"));
  //trgResultsProcess_ = iConfig.getParameter<edm::InputTag>("hltresults").process();
  //token_jets = consumes<std::vector<pat::Jet>>(iConfig.getParameter<edm::InputTag>("jets"));

  std::cout << "Constructor end" << std::endl;
}


JetTriggerAnalyzer::~JetTriggerAnalyzer(){}


//
// member functions
//

// ------------ method called for each event  ------------
void
JetTriggerAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup){

  std::cout << "Analyze begin zero" << std::endl;


  using namespace edm;

  evt = iEvent.id().event();
  run = iEvent.id().run();
  lumi = iEvent.luminosityBlock();

  std::cout << "Analyze begin" << std::endl;

  ///////////////////////////////////////
  // Checking original trigger results //
  ///////////////////////////////////////

  edm::Handle<edm::TriggerResults> hltresults;
  iEvent.getByToken(token_hlt,hltresults);
  edm::TriggerNames const& triggerNames = iEvent.triggerNames(*hltresults);

  //bool cfg_changed = true;
  //hltPrescaleProvider_.init(iEvent.getRun(), iSetup, trgResultsProcess_, cfg_changed);
  //HLTConfigProvider const&  hltCfg = hltPrescaleProvider_.hltConfigProvider();

  int ntrigs = hltresults->size();
  //std::set<int> indices;

  //std::cout << "a" << std::endl;

  if(firstEvent){
    int n = 0;
    for(int itrig = 0; itrig != ntrigs; ++itrig) {
      TString trigName = triggerNames.triggerName(itrig);
      if(trigName.BeginsWith("HLT_DiPFJet") || trigName.BeginsWith("HLT_AK8PFJet")){
        Events->Branch(trigName+"_acc", trigflag+n, trigName+"_acc/I");
        //Events->Branch(trigName+"_psc", prscflag+n, trigName+"_psc/I");
        //indices.insert(itrig);
        n++;
      }
    }
    //firstEvent = false;
  }

  //bool save = false;
  int n = 0;
  for(int itrig = 0; itrig != ntrigs; ++itrig){
    TString trigName = triggerNames.triggerName(itrig);
    if(trigName.BeginsWith("HLT_DiPFJet") || trigName.BeginsWith("HLT_AK8PFJet")){
      trigflag[n] = (hltresults->accept(itrig)) ? 1 : 0;
      //prscflag[n] = (hltCfg.moduleType(hltCfg.moduleLabel(itrig,hltresults->index(itrig)))=="HLTPrescaler") ? 1 : 0;
      //if(!prscflag[n])
      //  save = true;
      n++;
    }
  }

  /////////////////////////////////
  // Checking re-run HLT results //
  /////////////////////////////////

  /*#edm::Handle<edm::TriggerResults> hltresults_rerun;
  iEvent.getByToken(token_hlt_rerun,hltresults_rerun);
  edm::TriggerNames const& triggerNames_rerun = iEvent.triggerNames(*hltresults_rerun);

  //bool cfg_changed = true;
  //hltPrescaleProvider_.init(iEvent.getRun(), iSetup, trgResultsProcess_, cfg_changed);
  //HLTConfigProvider const&  hltCfg = hltPrescaleProvider_.hltConfigProvider();

  int ntrigs_rerun = hltresults_rerun->size();
  //std::set<int> indices;

  //std::cout << "a" << std::endl;

  if(firstEvent){
    int n = 0;
    for(int itrig = 0; itrig != ntrigs_rerun; ++itrig) {
      TString trigName = triggerNames_rerun.triggerName(itrig);
      if(trigName.BeginsWith("HLT_DiPFJet") || trigName.BeginsWith("HLT_AK8PFJet")){
        Events->Branch(trigName+"_rerun_acc", trigflag_rerun+n, trigName+"_rerun_acc/I");
        //Events->Branch(trigName+"_psc", prscflag+n, trigName+"_psc/I");
        //indices.insert(itrig);
        n++;
      }
    }
    firstEvent = false;
  }

  //bool save = false;
  int n_rerun = 0;
  for(int itrig = 0; itrig != ntrigs_rerun; ++itrig){
    TString trigName = triggerNames_rerun.triggerName(itrig);
    if(trigName.BeginsWith("HLT_DiPFJet") || trigName.BeginsWith("HLT_AK8PFJet")){
      trigflag_rerun[n_rerun] = (hltresults_rerun->accept(itrig)) ? 1 : 0;
      //prscflag[n] = (hltCfg.moduleType(hltCfg.moduleLabel(itrig,hltresults->index(itrig)))=="HLTPrescaler") ? 1 : 0;
      //if(!prscflag[n])
      //  save = true;
      n_rerun++;
    }
  }
  */


  //std::cout << "b" << std::endl;
  // Get jets
  /*jetmult = 0; 
  
  edm::Handle<std::vector<pat::Jet>> jets;
  iEvent.getByToken(token_jets,jets);

  double leading = 0;

  for(auto jet = jets->begin(); jet != jets->end(); ++jet){
    pt[jetmult] = jet->pt();
    eta[jetmult] = jet->eta();
    phi[jetmult] = jet->phi();

    if(jet->pt() > leading)
      leading = jet->pt();

    jetmult++;
  } 

  //std::cout << "c" << std::endl;
  if(jetmult >= 1)*/
    Events->Fill();
}


// ------------ method called once each job just before starting event loop  ------------
void 
JetTriggerAnalyzer::beginJob(){
  edm::Service<TFileService> fs;
  Events = fs->make<TTree>("Events", "");
  Events->Branch("evt",&evt,"evt/I");
  Events->Branch("run",&run,"run/I");
  Events->Branch("lumi",&lumi,"lumi/I");

  Events->Branch("jetmult",&jetmult,"jetmult/I");
  Events->Branch("pt",pt,"pt[jetmult]/F");
  Events->Branch("eta",eta,"eta[jetmult]/F");
  Events->Branch("phi",phi,"phi[jetmult]/F");
 
  firstEvent = true;

  const int Max = 200;
  trigflag = new int[Max];
  //#trigflag_rerun = new int[Max];

  //prscflag = new int[Max];
}

// ------------ method called once each job just after ending the event loop  ------------
void 
JetTriggerAnalyzer::endJob(){
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
JetTriggerAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(JetTriggerAnalyzer);
