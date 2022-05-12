/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"

// Default Network Topology
//
//
//   CC1   CC2   CC3   CC4   CC5   CC6   CC7   CC8
//     |     |     |     |     |     |     |     |
//   =============================================
//                 LAN 192.168.1.0


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("labeval");

int main (int argc, char *argv[])
{

  CommandLine cmd (__FILE__);

  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer csmaNodes;
  csmaNodes.Create (8);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  // csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  InternetStackHelper stack;
  stack.Install (csmaNodes);

  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (2));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (102.0));

  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (2), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (10000));
  echoClient.SetAttribute ("Interval", TimeValue (MilliSeconds (200)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (csmaNodes.Get (4));
  clientApps.Start (Seconds (1.0));
  clientApps.Stop (Seconds (102.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  csma.EnablePcap ("lab", csmaDevices.Get (2), true);

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll(ascii.CreateFileStream("lab.tr"));

  AnimationInterface anim("lab.xml");
  anim.SetConstantPosition(csmaNodes.Get(0), 10.0,10.0);
  anim.SetConstantPosition(csmaNodes.Get(1), 15.0,10.0);
  anim.SetConstantPosition(csmaNodes.Get(2), 20.0,10.0);
  anim.SetConstantPosition(csmaNodes.Get(3), 25.0,10.0);

  anim.SetConstantPosition(csmaNodes.Get(4), 10.0,20.0);
  anim.SetConstantPosition(csmaNodes.Get(5), 15.0,20.0);
  anim.SetConstantPosition(csmaNodes.Get(6), 20.0,20.0);
  anim.SetConstantPosition(csmaNodes.Get(7), 25.0,20.0);

  Simulator::Stop (Seconds (100));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
