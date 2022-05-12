#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/ssid.h"
#include "ns3/netanim-module.h"
#include "ns3/error-model.h"


// Network Topology
//                                                               n1  n2  n3  n4
//         172.16.21.0                        172.16.25.0         |   |   |   |
//      ftp----------|                       |---------- r3 ===================
//            p2p    |      172.16.24.0      |   p2p           LAN 172.16.23.0
//                  r1 ----------------- r2
//            p2p    |   point-to-point   |   p2p            Wifi 172.16.27.0
//      cbr----------|                    |---------- wap ___*
//         172.16.22.0                        172.16.27.0      |   |   |
//                                                            w1  w2  w3
//

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Assg8");

int main() {

    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(13);

    NodeContainer ftp_r0 = NodeContainer(nodes.Get(0), nodes.Get(2));
    NodeContainer cbr_r0 = NodeContainer(nodes.Get(1), nodes.Get(2));
    NodeContainer cnc_ccc = NodeContainer(nodes.Get(2), nodes.Get(3));
    NodeContainer ccc_help_p2p = NodeContainer(nodes.Get(3), nodes.Get(4));
    NodeContainer help_lan = NodeContainer(nodes.Get(4), nodes.Get(5), nodes.Get(6), nodes.Get(7), nodes.Get(8));
    NodeContainer ccc_ap = NodeContainer(nodes.Get(3), nodes.Get(9));
    NodeContainer ap = NodeContainer(nodes.Get(9));
    NodeContainer laptops = NodeContainer(nodes.Get(10), nodes.Get(11), nodes.Get(12));

    PointToPointHelper p2p5;
    p2p5.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    NetDeviceContainer ftp_dev = p2p5.Install(ftp_r0);
    NetDeviceContainer cbr_dev = p2p5.Install(cbr_r0);
    NetDeviceContainer ccc_help_dev = p2p5.Install(ccc_help_p2p);
    NetDeviceContainer ccc_ap_dev = p2p5.Install(ccc_ap);

    PointToPointHelper p2p2;
    p2p2.SetDeviceAttribute("DataRate", StringValue("2Mbps"));
    p2p2.SetChannelAttribute("Delay", StringValue("20ms"));
    NetDeviceContainer cnc_ccc_dev = p2p2.Install(cnc_ccc);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("25Mbps"));
    NetDeviceContainer help_lan_dev = csma.Install(help_lan);

    // error rate of 0.15% at CCC Help desk
    Ptr<RateErrorModel> errmodel = CreateObject<RateErrorModel> ();
    errmodel->SetAttribute ("ErrorRate", DoubleValue (0.15));
    RateErrorModel::ErrorUnit e = RateErrorModel::ERROR_UNIT_PACKET;
    errmodel->SetUnit(e);
    help_lan_dev.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (errmodel));

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy = YansWifiPhyHelper();
    phy.SetChannel (channel.Create ());

    WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

    WifiMacHelper mac;
    Ssid ssid = Ssid ("DSS-genie");
    mac.SetType ("ns3::StaWifiMac",
                "Ssid", SsidValue (ssid),
                "ActiveProbing", BooleanValue (false));

    NetDeviceContainer laptop_dev;
    laptop_dev = wifi.Install (phy, mac, laptops);

    mac.SetType ("ns3::ApWifiMac",
                "Ssid", SsidValue (ssid));

    NetDeviceContainer ap_dev;
    ap_dev = wifi.Install (phy, mac, ap);

    MobilityHelper mobility;

    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                    "MinX", DoubleValue (0.0),
                                    "MinY", DoubleValue (0.0),
                                    "DeltaX", DoubleValue (5.0),
                                    "DeltaY", DoubleValue (10.0),
                                    "GridWidth", UintegerValue (3),
                                    "LayoutType", StringValue ("RowFirst"));

    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                                "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
    mobility.Install (laptops);

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (ap);

    InternetStackHelper stack;
    stack.Install (nodes);

    Ipv4AddressHelper address;
    address.SetBase ("172.16.21.0", "255.255.255.0");
    Ipv4InterfaceContainer ftp_ip = address.Assign (ftp_dev);

    address.SetBase ("172.16.22.0", "255.255.255.0");
    Ipv4InterfaceContainer cbr_ip = address.Assign (cbr_dev);

    address.SetBase ("172.16.23.0", "255.255.255.0");
    Ipv4InterfaceContainer help_ip = address.Assign (help_lan_dev);

    address.SetBase ("172.16.24.0", "255.255.255.0");
    Ipv4InterfaceContainer cnc_ccc_ip = address.Assign (cnc_ccc_dev);

    address.SetBase ("172.16.25.0", "255.255.255.0");
    Ipv4InterfaceContainer ccc_help_ip = address.Assign (ccc_help_dev);

    address.SetBase ("172.16.26.0", "255.255.255.0");
    Ipv4InterfaceContainer ccc_ap_ip = address.Assign (ccc_ap_dev);

    address.SetBase ("172.16.27.0", "255.255.255.0");
    Ipv4InterfaceContainer wifi_if = address.Assign (ap_dev);
    address.Assign(laptop_dev);

    // Create FTP server at server 1
    BulkSendHelper source ("ns3::TcpSocketFactory",
                            InetSocketAddress (help_ip.GetAddress (1), 21));
    source.SetAttribute ("MaxBytes", UintegerValue (0));
    ApplicationContainer sourceApps = source.Install (ftp_r0.Get (0));
    sourceApps.Start (Seconds (0));
    sourceApps.Stop (Seconds (100));

    // Create CBR server
    UdpEchoServerHelper CBRServer (9);
    ApplicationContainer CBRServerApps = CBRServer.Install (cbr_r0.Get(0));
    CBRServerApps.Start (Seconds (0));
    CBRServerApps.Stop (Seconds (100));

    // Create FTP clients at CCC Help desk
    PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), 21));
    ApplicationContainer sinkApps = sink.Install (help_lan);
    sinkApps.Start (Seconds (0));
    sinkApps.Stop (Seconds (100));

    // Create CBR clients at DSS wifi
    UdpEchoClientHelper CBRClient (cbr_ip.GetAddress (0), 9);
    CBRClient.SetAttribute ("MaxPackets", UintegerValue (100));
    CBRClient.SetAttribute ("Interval", TimeValue (MilliSeconds (200)));
    CBRClient.SetAttribute ("PacketSize", UintegerValue (1024));

    // Install UdpEchoClient in wifi nodes
    ApplicationContainer clientWifiApps = CBRClient.Install (laptops);
    clientWifiApps.Start (Seconds (0));
    clientWifiApps.Stop (Seconds (100));


    //For routers to be able to forward packets, they need to have routing rules.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    p2p5.EnablePcap("ftpserver", ftp_r0);
    p2p5.EnablePcap("cbrserver", cbr_r0);

    AsciiTraceHelper ascii;
    p2p2.EnableAsciiAll(ascii.CreateFileStream("cnc-ccc.tr"));

    AnimationInterface anim("assg8.xml");
    anim.SetConstantPosition(nodes.Get(0), 10.0,10.0);
    anim.SetConstantPosition(nodes.Get(1), 10.0,20.0);

    anim.SetConstantPosition(nodes.Get(2), 15.0,15.0);
    anim.SetConstantPosition(nodes.Get(3), 20.0,15.0);
    anim.SetConstantPosition(nodes.Get(4), 25.0,20.0);

    anim.SetConstantPosition(nodes.Get(5), 27.0,25.0);
    anim.SetConstantPosition(nodes.Get(6), 30.0,25.0);
    anim.SetConstantPosition(nodes.Get(7), 27.0,15.0);
    anim.SetConstantPosition(nodes.Get(8), 30.0,15.0);

    anim.SetConstantPosition(laptops.Get(0), 30.0,9.0);
    anim.SetConstantPosition(laptops.Get(1), 30.0,7.0);
    anim.SetConstantPosition(laptops.Get(2), 30.0,5.0);

    anim.SetConstantPosition(ap.Get(0), 27.5,7.0);

    Simulator::Stop (Seconds (100));
    Simulator::Run ();

    Simulator::Destroy ();
    return 0;
}