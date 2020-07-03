//examples-ramsha.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

namespace ns3 {

int
main(int argc, char* argv[])
{
  CommandLine cmd;
  cmd.Parse(argc, argv);

  AnnotatedTopologyReader topologyReader("", 1);
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/topo-ramsha.txt");
  topologyReader.Read();

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/broadcast");

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // Getting containers for the consumer/producer
  Ptr<Node> consumers[3] = {Names::Find<Node>("Client"), Names::Find<Node>("P2P"),
                            Names::Find<Node>("Intra-PoP")};
  Ptr<Node> producer = Names::Find<Node>("Server");

  for (int i = 0; i < 3; i++) {
    ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
    consumerHelper.SetAttribute("Frequency", StringValue("100")); // 100 interests a second

    // Each consumer will express unique interests /root/<leaf-name>/<seq-no>
    consumerHelper.SetPrefix("/Server/" + Names::FindName(consumers[i]));
    consumerHelper.Install(consumers[i]);
  }

  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));

  // Register /root prefix with global routing controller and
  // install producer that will satisfy Interests in /Server namespace
  ndnGlobalRoutingHelper.AddOrigins("/Server", producer);
  producerHelper.SetPrefix("/Server");
  producerHelper.Install(producer);

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop(Seconds(20.0));

  ndn::AppDelayTracer::InstallAll("app-delays-trace.txt");

  ndn::L3RateTracer::InstallAll("rate-trace.txt", Seconds(0.5));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}