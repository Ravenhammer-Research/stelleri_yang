#pragma once

#include <string>
#include <unordered_map>

namespace yang {

// Enum representing IANA `ifType` identities. C++ identifiers use
// '_' in place of any '-' characters from the YANG names.
enum class IanaIfType {
	Unknown,
	other,
	regular1822,
	hdh1822,
	ddnX25,
	rfc877x25,
	ethernetCsmacd,
	iso88023Csmacd,
	iso88024TokenBus,
	iso88025TokenRing,
	iso88026Man,
	starLan,
	proteon10Mbit,
	proteon80Mbit,
	hyperchannel,
	fddi,
	lapb,
	sdlc,
	ds1,
	e1,
	basicISDN,
	primaryISDN,
	propPointToPointSerial,
	ppp,
	softwareLoopback,
	eon,
	ethernet3Mbit,
	nsip,
	slip,
	ultra,
	ds3,
	sip,
	frameRelay,
	rs232,
	para,
	arcnet,
	arcnetPlus,
	miox25,
	sonet,
	x25ple,
	iso88022llc,
	localTalk,
	smdsDxi,
	frameRelayService,
	v35,
	hssi,
	hippi,
	modem,
	aal5,
	sonetPath,
	sonetVT,
	smdsIcip,
	propVirtual,
	propMultiplexor,
	ieee80212,
	fibreChannel,
	hippiInterface,
	frameRelayInterconnect,
	aflane8023,
	aflane8025,
	cctEmul,
	fastEther,
	isdn,
	v11,
	v36,
	g703at64k,
	g703at2mb,
	qllc,
	fastEtherFX,
	channel,
	ieee80211,
	ibm370parChan,
	escon,
	dlsw,
	isdns,
	isdnu,
	lapd,
	ipSwitch,
	rsrb,
	atmLogical,
	ds0,
	ds0Bundle,
	bsc,
	async,
	cnr,
	iso88025Dtr,
	eplrs,
	arap,
	propCnls,
	hostPad,
	termPad,
	frameRelayMPI,
	x213,
	adsl,
	radsl,
	sdsl,
	vdsl,
	iso88025CRFPInt,
	myrinet,
	voiceEM,
	voiceFXO,
	voiceFXS,
	voiceEncap,
	voiceOverIp,
	atmDxi,
	atmFuni,
	atmIma,
	pppMultilinkBundle,
	ipOverCdlc,
	ipOverClaw,
	stackToStack,
	virtualIpAddress,
	mpc,
	ipOverAtm,
	iso88025Fiber,
	tdlc,
	gigabitEthernet,
	hdlc,
	lapf,
	v37,
	x25mlp,
	x25huntGroup,
	transpHdlc,
	interleave,
	fast,
	ip,
	docsCableMaclayer,
	docsCableDownstream,
	docsCableUpstream,
	a12MppSwitch,
	tunnel,
	coffee,
	ces,
	atmSubInterface,
	l2vlan,
	l3ipvlan,
	l3ipxvlan,
	digitalPowerline,
	mediaMailOverIp,
	dtm,
	dcn,
	ipForward,
	msdsl,
	ieee1394,
	if_gsn,
	dvbRccMacLayer,
	dvbRccDownstream,
	dvbRccUpstream,
	atmVirtual,
	mplsTunnel,
	srp,
	voiceOverAtm,
	voiceOverFrameRelay,
	idsl,
	compositeLink,
	ss7SigLink,
	propWirelessP2P,
	frForward,
	rfc1483,
	usb,
	ieee8023adLag,
	bgppolicyaccounting,
	frf16MfrBundle,
	h323Gatekeeper,
	h323Proxy,
	mpls,
	mfSigLink,
	hdsl2,
	shdsl,
	ds1FDL,
	pos,
	dvbAsiIn,
	dvbAsiOut,
	plc,
	nfas,
	tr008,
	gr303RDT,
	gr303IDT,
	isup,
	propDocsWirelessMaclayer,
	propDocsWirelessDownstream,
	propDocsWirelessUpstream,
	hiperlan2,
	propBWAp2Mp,
	sonetOverheadChannel,
	digitalWrapperOverheadChannel,
	aal2,
	radioMAC,
	atmRadio,
	imt,
	mvl,
	reachDSL,
	frDlciEndPt,
	atmVciEndPt,
	opticalChannel,
	opticalTransport,
	propAtm,
	voiceOverCable,
	infiniband,
	teLink,
	q2931,
	virtualTg,
	sipTg,
	sipSig,
	docsCableUpstreamChannel,
	econet,
	pon155,
	pon622,
	bridge,
	linegroup,
	voiceEMFGD,
	voiceFGDEANA,
	voiceDID,
	mpegTransport,
	sixToFour,
	gtp,
	pdnEtherLoop1,
	pdnEtherLoop2,
	opticalChannelGroup,
	homepna,
	gfp,
	ciscoISLvlan,
	actelisMetaLOOP,
	fcipLink,
	rpr,
	qam,
	lmp,
	cblVectaStar,
	docsCableMCmtsDownstream,
	adsl2,
	macSecControlledIF,
	macSecUncontrolledIF,
	aviciOpticalEther,
	atmbond,
	voiceFGDOS,
	mocaVersion1,
	ieee80216WMAN,
	adsl2plus,
	dvbRcsMacLayer,
	dvbTdm,
	dvbRcsTdma,
	x86Laps,
	wwanPP,
	wwanPP2,
	voiceEBS,
	ifPwType,
	ilan,
	pip,
	aluELP,
	gpon,
	vdsl2,
	capwapDot11Profile,
	capwapDot11Bss,
	capwapWtpVirtualRadio,
	bits,
	docsCableUpstreamRfPort,
	cableDownstreamRfPort,
	vmwareVirtualNic,
	ieee802154,
	otnOdu,
	otnOtu,
	ifVfiType,
	g9981,
	g9982,
	g9983,
	aluEpon,
	aluEponOnu,
	aluEponPhysicalUni,
	aluEponLogicalLink,
	aluGponOnu,
	aluGponPhysicalUni,
	vmwareNicTeam,
	docsOfdmDownstream,
	docsOfdmaUpstream,
	gfast,
	sdci,
	xboxWireless,
	fastdsl,
	docsCableScte55d1FwdOob,
	docsCableScte55d1RetOob,
	docsCableScte55d2DsOob,
	docsCableScte55d2UsOob,
	docsCableNdf,
	docsCableNdr,
	ptm,
	ghn,
	otnOtsi,
	otnOtuc,
	otnOduc,
	otnOtsig,
	microwaveCarrierTermination,
	microwaveRadioLinkTerminal,
	ieee8021axDrni,
	ax25,
	ieee19061nanocom,
	cpri,
	omni,
	roe,
	p2pOverLan
};

// Convert enum to the canonical YANG identity string.
inline std::string toString(IanaIfType t) {
	switch (t) {
		case IanaIfType::other: return "other";
		case IanaIfType::regular1822: return "regular1822";
		case IanaIfType::hdh1822: return "hdh1822";
		case IanaIfType::ddnX25: return "ddnX25";
		case IanaIfType::rfc877x25: return "rfc877x25";
		case IanaIfType::ethernetCsmacd: return "ethernetCsmacd";
		case IanaIfType::iso88023Csmacd: return "iso88023Csmacd";
		case IanaIfType::iso88024TokenBus: return "iso88024TokenBus";
		case IanaIfType::iso88025TokenRing: return "iso88025TokenRing";
		case IanaIfType::iso88026Man: return "iso88026Man";
		case IanaIfType::starLan: return "starLan";
		case IanaIfType::proteon10Mbit: return "proteon10Mbit";
		case IanaIfType::proteon80Mbit: return "proteon80Mbit";
		case IanaIfType::hyperchannel: return "hyperchannel";
		case IanaIfType::fddi: return "fddi";
		case IanaIfType::lapb: return "lapb";
		case IanaIfType::sdlc: return "sdlc";
		case IanaIfType::ds1: return "ds1";
		case IanaIfType::e1: return "e1";
		case IanaIfType::basicISDN: return "basicISDN";
		case IanaIfType::primaryISDN: return "primaryISDN";
		case IanaIfType::propPointToPointSerial: return "propPointToPointSerial";
		case IanaIfType::ppp: return "ppp";
		case IanaIfType::softwareLoopback: return "softwareLoopback";
		case IanaIfType::eon: return "eon";
		case IanaIfType::ethernet3Mbit: return "ethernet3Mbit";
		case IanaIfType::nsip: return "nsip";
		case IanaIfType::slip: return "slip";
		case IanaIfType::ultra: return "ultra";
		case IanaIfType::ds3: return "ds3";
		case IanaIfType::sip: return "sip";
		case IanaIfType::frameRelay: return "frameRelay";
		case IanaIfType::rs232: return "rs232";
		case IanaIfType::para: return "para";
		case IanaIfType::arcnet: return "arcnet";
		case IanaIfType::arcnetPlus: return "arcnetPlus";
		case IanaIfType::miox25: return "miox25";
		case IanaIfType::sonet: return "sonet";
		case IanaIfType::x25ple: return "x25ple";
		case IanaIfType::iso88022llc: return "iso88022llc";
		case IanaIfType::localTalk: return "localTalk";
		case IanaIfType::smdsDxi: return "smdsDxi";
		case IanaIfType::frameRelayService: return "frameRelayService";
		case IanaIfType::v35: return "v35";
		case IanaIfType::hssi: return "hssi";
		case IanaIfType::hippi: return "hippi";
		case IanaIfType::modem: return "modem";
		case IanaIfType::aal5: return "aal5";
		case IanaIfType::sonetPath: return "sonetPath";
		case IanaIfType::sonetVT: return "sonetVT";
		case IanaIfType::smdsIcip: return "smdsIcip";
		case IanaIfType::propVirtual: return "propVirtual";
		case IanaIfType::propMultiplexor: return "propMultiplexor";
		case IanaIfType::ieee80212: return "ieee80212";
		case IanaIfType::fibreChannel: return "fibreChannel";
		case IanaIfType::hippiInterface: return "hippiInterface";
		case IanaIfType::frameRelayInterconnect: return "frameRelayInterconnect";
		case IanaIfType::aflane8023: return "aflane8023";
		case IanaIfType::aflane8025: return "aflane8025";
		case IanaIfType::cctEmul: return "cctEmul";
		case IanaIfType::fastEther: return "fastEther";
		case IanaIfType::isdn: return "isdn";
		case IanaIfType::v11: return "v11";
		case IanaIfType::v36: return "v36";
		case IanaIfType::g703at64k: return "g703at64k";
		case IanaIfType::g703at2mb: return "g703at2mb";
		case IanaIfType::qllc: return "qllc";
		case IanaIfType::fastEtherFX: return "fastEtherFX";
		case IanaIfType::channel: return "channel";
		case IanaIfType::ieee80211: return "ieee80211";
		case IanaIfType::ibm370parChan: return "ibm370parChan";
		case IanaIfType::escon: return "escon";
		case IanaIfType::dlsw: return "dlsw";
		case IanaIfType::isdns: return "isdns";
		case IanaIfType::isdnu: return "isdnu";
		case IanaIfType::lapd: return "lapd";
		case IanaIfType::ipSwitch: return "ipSwitch";
		case IanaIfType::rsrb: return "rsrb";
		case IanaIfType::atmLogical: return "atmLogical";
		case IanaIfType::ds0: return "ds0";
		case IanaIfType::ds0Bundle: return "ds0Bundle";
		case IanaIfType::bsc: return "bsc";
		case IanaIfType::async: return "async";
		case IanaIfType::cnr: return "cnr";
		case IanaIfType::iso88025Dtr: return "iso88025Dtr";
		case IanaIfType::eplrs: return "eplrs";
		case IanaIfType::arap: return "arap";
		case IanaIfType::propCnls: return "propCnls";
		case IanaIfType::hostPad: return "hostPad";
		case IanaIfType::termPad: return "termPad";
		case IanaIfType::frameRelayMPI: return "frameRelayMPI";
		case IanaIfType::x213: return "x213";
		case IanaIfType::adsl: return "adsl";
		case IanaIfType::radsl: return "radsl";
		case IanaIfType::sdsl: return "sdsl";
		case IanaIfType::vdsl: return "vdsl";
		case IanaIfType::iso88025CRFPInt: return "iso88025CRFPInt";
		case IanaIfType::myrinet: return "myrinet";
		case IanaIfType::voiceEM: return "voiceEM";
		case IanaIfType::voiceFXO: return "voiceFXO";
		case IanaIfType::voiceFXS: return "voiceFXS";
		case IanaIfType::voiceEncap: return "voiceEncap";
		case IanaIfType::voiceOverIp: return "voiceOverIp";
		case IanaIfType::atmDxi: return "atmDxi";
		case IanaIfType::atmFuni: return "atmFuni";
		case IanaIfType::atmIma: return "atmIma";
		case IanaIfType::pppMultilinkBundle: return "pppMultilinkBundle";
		case IanaIfType::ipOverCdlc: return "ipOverCdlc";
		case IanaIfType::ipOverClaw: return "ipOverClaw";
		case IanaIfType::stackToStack: return "stackToStack";
		case IanaIfType::virtualIpAddress: return "virtualIpAddress";
		case IanaIfType::mpc: return "mpc";
		case IanaIfType::ipOverAtm: return "ipOverAtm";
		case IanaIfType::iso88025Fiber: return "iso88025Fiber";
		case IanaIfType::tdlc: return "tdlc";
		case IanaIfType::gigabitEthernet: return "gigabitEthernet";
		case IanaIfType::hdlc: return "hdlc";
		case IanaIfType::lapf: return "lapf";
		case IanaIfType::v37: return "v37";
		case IanaIfType::x25mlp: return "x25mlp";
		case IanaIfType::x25huntGroup: return "x25huntGroup";
		case IanaIfType::transpHdlc: return "transpHdlc";
		case IanaIfType::interleave: return "interleave";
		case IanaIfType::fast: return "fast";
		case IanaIfType::ip: return "ip";
		case IanaIfType::docsCableMaclayer: return "docsCableMaclayer";
		case IanaIfType::docsCableDownstream: return "docsCableDownstream";
		case IanaIfType::docsCableUpstream: return "docsCableUpstream";
		case IanaIfType::a12MppSwitch: return "a12MppSwitch";
		case IanaIfType::tunnel: return "tunnel";
		case IanaIfType::coffee: return "coffee";
		case IanaIfType::ces: return "ces";
		case IanaIfType::atmSubInterface: return "atmSubInterface";
		case IanaIfType::l2vlan: return "l2vlan";
		case IanaIfType::l3ipvlan: return "l3ipvlan";
		case IanaIfType::l3ipxvlan: return "l3ipxvlan";
		case IanaIfType::digitalPowerline: return "digitalPowerline";
		case IanaIfType::mediaMailOverIp: return "mediaMailOverIp";
		case IanaIfType::dtm: return "dtm";
		case IanaIfType::dcn: return "dcn";
		case IanaIfType::ipForward: return "ipForward";
		case IanaIfType::msdsl: return "msdsl";
		case IanaIfType::ieee1394: return "ieee1394";
		case IanaIfType::if_gsn: return "if-gsn";
		case IanaIfType::dvbRccMacLayer: return "dvbRccMacLayer";
		case IanaIfType::dvbRccDownstream: return "dvbRccDownstream";
		case IanaIfType::dvbRccUpstream: return "dvbRccUpstream";
		case IanaIfType::atmVirtual: return "atmVirtual";
		case IanaIfType::mplsTunnel: return "mplsTunnel";
		case IanaIfType::srp: return "srp";
		case IanaIfType::voiceOverAtm: return "voiceOverAtm";
		case IanaIfType::voiceOverFrameRelay: return "voiceOverFrameRelay";
		case IanaIfType::idsl: return "idsl";
		case IanaIfType::compositeLink: return "compositeLink";
		case IanaIfType::ss7SigLink: return "ss7SigLink";
		case IanaIfType::propWirelessP2P: return "propWirelessP2P";
		case IanaIfType::frForward: return "frForward";
		case IanaIfType::rfc1483: return "rfc1483";
		case IanaIfType::usb: return "usb";
		case IanaIfType::ieee8023adLag: return "ieee8023adLag";
		case IanaIfType::bgppolicyaccounting: return "bgppolicyaccounting";
		case IanaIfType::frf16MfrBundle: return "frf16MfrBundle";
		case IanaIfType::h323Gatekeeper: return "h323Gatekeeper";
		case IanaIfType::h323Proxy: return "h323Proxy";
		case IanaIfType::mpls: return "mpls";
		case IanaIfType::mfSigLink: return "mfSigLink";
		case IanaIfType::hdsl2: return "hdsl2";
		case IanaIfType::shdsl: return "shdsl";
		case IanaIfType::ds1FDL: return "ds1FDL";
		case IanaIfType::pos: return "pos";
		case IanaIfType::dvbAsiIn: return "dvbAsiIn";
		case IanaIfType::dvbAsiOut: return "dvbAsiOut";
		case IanaIfType::plc: return "plc";
		case IanaIfType::nfas: return "nfas";
		case IanaIfType::tr008: return "tr008";
		case IanaIfType::gr303RDT: return "gr303RDT";
		case IanaIfType::gr303IDT: return "gr303IDT";
		case IanaIfType::isup: return "isup";
		case IanaIfType::propDocsWirelessMaclayer: return "propDocsWirelessMaclayer";
		case IanaIfType::propDocsWirelessDownstream: return "propDocsWirelessDownstream";
		case IanaIfType::propDocsWirelessUpstream: return "propDocsWirelessUpstream";
		case IanaIfType::hiperlan2: return "hiperlan2";
		case IanaIfType::propBWAp2Mp: return "propBWAp2Mp";
		case IanaIfType::sonetOverheadChannel: return "sonetOverheadChannel";
		case IanaIfType::digitalWrapperOverheadChannel: return "digitalWrapperOverheadChannel";
		case IanaIfType::aal2: return "aal2";
		case IanaIfType::radioMAC: return "radioMAC";
		case IanaIfType::atmRadio: return "atmRadio";
		case IanaIfType::imt: return "imt";
		case IanaIfType::mvl: return "mvl";
		case IanaIfType::reachDSL: return "reachDSL";
		case IanaIfType::frDlciEndPt: return "frDlciEndPt";
		case IanaIfType::atmVciEndPt: return "atmVciEndPt";
		case IanaIfType::opticalChannel: return "opticalChannel";
		case IanaIfType::opticalTransport: return "opticalTransport";
		case IanaIfType::propAtm: return "propAtm";
		case IanaIfType::voiceOverCable: return "voiceOverCable";
		case IanaIfType::infiniband: return "infiniband";
		case IanaIfType::teLink: return "teLink";
		case IanaIfType::q2931: return "q2931";
		case IanaIfType::virtualTg: return "virtualTg";
		case IanaIfType::sipTg: return "sipTg";
		case IanaIfType::sipSig: return "sipSig";
		case IanaIfType::docsCableUpstreamChannel: return "docsCableUpstreamChannel";
		case IanaIfType::econet: return "econet";
		case IanaIfType::pon155: return "pon155";
		case IanaIfType::pon622: return "pon622";
		case IanaIfType::bridge: return "bridge";
		case IanaIfType::linegroup: return "linegroup";
		case IanaIfType::voiceEMFGD: return "voiceEMFGD";
		case IanaIfType::voiceFGDEANA: return "voiceFGDEANA";
		case IanaIfType::voiceDID: return "voiceDID";
		case IanaIfType::mpegTransport: return "mpegTransport";
		case IanaIfType::sixToFour: return "sixToFour";
		case IanaIfType::gtp: return "gtp";
		case IanaIfType::pdnEtherLoop1: return "pdnEtherLoop1";
		case IanaIfType::pdnEtherLoop2: return "pdnEtherLoop2";
		case IanaIfType::opticalChannelGroup: return "opticalChannelGroup";
		case IanaIfType::homepna: return "homepna";
		case IanaIfType::gfp: return "gfp";
		case IanaIfType::ciscoISLvlan: return "ciscoISLvlan";
		case IanaIfType::actelisMetaLOOP: return "actelisMetaLOOP";
		case IanaIfType::fcipLink: return "fcipLink";
		case IanaIfType::rpr: return "rpr";
		case IanaIfType::qam: return "qam";
		case IanaIfType::lmp: return "lmp";
		case IanaIfType::cblVectaStar: return "cblVectaStar";
		case IanaIfType::docsCableMCmtsDownstream: return "docsCableMCmtsDownstream";
		case IanaIfType::adsl2: return "adsl2";
		case IanaIfType::macSecControlledIF: return "macSecControlledIF";
		case IanaIfType::macSecUncontrolledIF: return "macSecUncontrolledIF";
		case IanaIfType::aviciOpticalEther: return "aviciOpticalEther";
		case IanaIfType::atmbond: return "atmbond";
		case IanaIfType::voiceFGDOS: return "voiceFGDOS";
		case IanaIfType::mocaVersion1: return "mocaVersion1";
		case IanaIfType::ieee80216WMAN: return "ieee80216WMAN";
		case IanaIfType::adsl2plus: return "adsl2plus";
		case IanaIfType::dvbRcsMacLayer: return "dvbRcsMacLayer";
		case IanaIfType::dvbTdm: return "dvbTdm";
		case IanaIfType::dvbRcsTdma: return "dvbRcsTdma";
		case IanaIfType::x86Laps: return "x86Laps";
		case IanaIfType::wwanPP: return "wwanPP";
		case IanaIfType::wwanPP2: return "wwanPP2";
		case IanaIfType::voiceEBS: return "voiceEBS";
		case IanaIfType::ifPwType: return "ifPwType";
		case IanaIfType::ilan: return "ilan";
		case IanaIfType::pip: return "pip";
		case IanaIfType::aluELP: return "aluELP";
		case IanaIfType::gpon: return "gpon";
		case IanaIfType::vdsl2: return "vdsl2";
		case IanaIfType::capwapDot11Profile: return "capwapDot11Profile";
		case IanaIfType::capwapDot11Bss: return "capwapDot11Bss";
		case IanaIfType::capwapWtpVirtualRadio: return "capwapWtpVirtualRadio";
		case IanaIfType::bits: return "bits";
		case IanaIfType::docsCableUpstreamRfPort: return "docsCableUpstreamRfPort";
		case IanaIfType::cableDownstreamRfPort: return "cableDownstreamRfPort";
		case IanaIfType::vmwareVirtualNic: return "vmwareVirtualNic";
		case IanaIfType::ieee802154: return "ieee802154";
		case IanaIfType::otnOdu: return "otnOdu";
		case IanaIfType::otnOtu: return "otnOtu";
		case IanaIfType::ifVfiType: return "ifVfiType";
		case IanaIfType::g9981: return "g9981";
		case IanaIfType::g9982: return "g9982";
		case IanaIfType::g9983: return "g9983";
		case IanaIfType::aluEpon: return "aluEpon";
		case IanaIfType::aluEponOnu: return "aluEponOnu";
		case IanaIfType::aluEponPhysicalUni: return "aluEponPhysicalUni";
		case IanaIfType::aluEponLogicalLink: return "aluEponLogicalLink";
		case IanaIfType::aluGponOnu: return "aluGponOnu";
		case IanaIfType::aluGponPhysicalUni: return "aluGponPhysicalUni";
		case IanaIfType::vmwareNicTeam: return "vmwareNicTeam";
		case IanaIfType::docsOfdmDownstream: return "docsOfdmDownstream";
		case IanaIfType::docsOfdmaUpstream: return "docsOfdmaUpstream";
		case IanaIfType::gfast: return "gfast";
		case IanaIfType::sdci: return "sdci";
		case IanaIfType::xboxWireless: return "xboxWireless";
		case IanaIfType::fastdsl: return "fastdsl";
		case IanaIfType::docsCableScte55d1FwdOob: return "docsCableScte55d1FwdOob";
		case IanaIfType::docsCableScte55d1RetOob: return "docsCableScte55d1RetOob";
		case IanaIfType::docsCableScte55d2DsOob: return "docsCableScte55d2DsOob";
		case IanaIfType::docsCableScte55d2UsOob: return "docsCableScte55d2UsOob";
		case IanaIfType::docsCableNdf: return "docsCableNdf";
		case IanaIfType::docsCableNdr: return "docsCableNdr";
		case IanaIfType::ptm: return "ptm";
		case IanaIfType::ghn: return "ghn";
		case IanaIfType::otnOtsi: return "otnOtsi";
		case IanaIfType::otnOtuc: return "otnOtuc";
		case IanaIfType::otnOduc: return "otnOduc";
		case IanaIfType::otnOtsig: return "otnOtsig";
		case IanaIfType::microwaveCarrierTermination: return "microwaveCarrierTermination";
		case IanaIfType::microwaveRadioLinkTerminal: return "microwaveRadioLinkTerminal";
		case IanaIfType::ieee8021axDrni: return "ieee8021axDrni";
		case IanaIfType::ax25: return "ax25";
		case IanaIfType::ieee19061nanocom: return "ieee19061nanocom";
		case IanaIfType::cpri: return "cpri";
		case IanaIfType::omni: return "omni";
		case IanaIfType::roe: return "roe";
		case IanaIfType::p2pOverLan: return "p2pOverLan";
		default: return "unknown";
	}
}

// Convert a string (YANG identity) to the enum. Returns Unknown for
// unrecognized identity names. Matching is exact (case-sensitive).
inline IanaIfType fromString(const std::string &s) {
	static const std::unordered_map<std::string, IanaIfType> m = {
		{"other", IanaIfType::other},
		{"regular1822", IanaIfType::regular1822},
		{"hdh1822", IanaIfType::hdh1822},
		{"ddnX25", IanaIfType::ddnX25},
		{"rfc877x25", IanaIfType::rfc877x25},
		{"ethernetCsmacd", IanaIfType::ethernetCsmacd},
		{"iso88023Csmacd", IanaIfType::iso88023Csmacd},
		{"iso88024TokenBus", IanaIfType::iso88024TokenBus},
		{"iso88025TokenRing", IanaIfType::iso88025TokenRing},
		{"iso88026Man", IanaIfType::iso88026Man},
		{"starLan", IanaIfType::starLan},
		{"proteon10Mbit", IanaIfType::proteon10Mbit},
		{"proteon80Mbit", IanaIfType::proteon80Mbit},
		{"hyperchannel", IanaIfType::hyperchannel},
		{"fddi", IanaIfType::fddi},
		{"lapb", IanaIfType::lapb},
		{"sdlc", IanaIfType::sdlc},
		{"ds1", IanaIfType::ds1},
		{"e1", IanaIfType::e1},
		{"basicISDN", IanaIfType::basicISDN},
		{"primaryISDN", IanaIfType::primaryISDN},
		{"propPointToPointSerial", IanaIfType::propPointToPointSerial},
		{"ppp", IanaIfType::ppp},
		{"softwareLoopback", IanaIfType::softwareLoopback},
		{"eon", IanaIfType::eon},
		{"ethernet3Mbit", IanaIfType::ethernet3Mbit},
		{"nsip", IanaIfType::nsip},
		{"slip", IanaIfType::slip},
		{"ultra", IanaIfType::ultra},
		{"ds3", IanaIfType::ds3},
		{"sip", IanaIfType::sip},
		{"frameRelay", IanaIfType::frameRelay},
		{"rs232", IanaIfType::rs232},
		{"para", IanaIfType::para},
		{"arcnet", IanaIfType::arcnet},
		{"arcnetPlus", IanaIfType::arcnetPlus},
		{"miox25", IanaIfType::miox25},
		{"sonet", IanaIfType::sonet},
		{"x25ple", IanaIfType::x25ple},
		{"iso88022llc", IanaIfType::iso88022llc},
		{"localTalk", IanaIfType::localTalk},
		{"smdsDxi", IanaIfType::smdsDxi},
		{"frameRelayService", IanaIfType::frameRelayService},
		{"v35", IanaIfType::v35},
		{"hssi", IanaIfType::hssi},
		{"hippi", IanaIfType::hippi},
		{"modem", IanaIfType::modem},
		{"aal5", IanaIfType::aal5},
		{"sonetPath", IanaIfType::sonetPath},
		{"sonetVT", IanaIfType::sonetVT},
		{"smdsIcip", IanaIfType::smdsIcip},
		{"propVirtual", IanaIfType::propVirtual},
		{"propMultiplexor", IanaIfType::propMultiplexor},
		{"ieee80212", IanaIfType::ieee80212},
		{"fibreChannel", IanaIfType::fibreChannel},
		{"hippiInterface", IanaIfType::hippiInterface},
		{"frameRelayInterconnect", IanaIfType::frameRelayInterconnect},
		{"aflane8023", IanaIfType::aflane8023},
		{"aflane8025", IanaIfType::aflane8025},
		{"cctEmul", IanaIfType::cctEmul},
		{"fastEther", IanaIfType::fastEther},
		{"isdn", IanaIfType::isdn},
		{"v11", IanaIfType::v11},
		{"v36", IanaIfType::v36},
		{"g703at64k", IanaIfType::g703at64k},
		{"g703at2mb", IanaIfType::g703at2mb},
		{"qllc", IanaIfType::qllc},
		{"fastEtherFX", IanaIfType::fastEtherFX},
		{"channel", IanaIfType::channel},
		{"ieee80211", IanaIfType::ieee80211},
		{"ibm370parChan", IanaIfType::ibm370parChan},
		{"escon", IanaIfType::escon},
		{"dlsw", IanaIfType::dlsw},
		{"isdns", IanaIfType::isdns},
		{"isdnu", IanaIfType::isdnu},
		{"lapd", IanaIfType::lapd},
		{"ipSwitch", IanaIfType::ipSwitch},
		{"rsrb", IanaIfType::rsrb},
		{"atmLogical", IanaIfType::atmLogical},
		{"ds0", IanaIfType::ds0},
		{"ds0Bundle", IanaIfType::ds0Bundle},
		{"bsc", IanaIfType::bsc},
		{"async", IanaIfType::async},
		{"cnr", IanaIfType::cnr},
		{"iso88025Dtr", IanaIfType::iso88025Dtr},
		{"eplrs", IanaIfType::eplrs},
		{"arap", IanaIfType::arap},
		{"propCnls", IanaIfType::propCnls},
		{"hostPad", IanaIfType::hostPad},
		{"termPad", IanaIfType::termPad},
		{"frameRelayMPI", IanaIfType::frameRelayMPI},
		{"x213", IanaIfType::x213},
		{"adsl", IanaIfType::adsl},
		{"radsl", IanaIfType::radsl},
		{"sdsl", IanaIfType::sdsl},
		{"vdsl", IanaIfType::vdsl},
		{"iso88025CRFPInt", IanaIfType::iso88025CRFPInt},
		{"myrinet", IanaIfType::myrinet},
		{"voiceEM", IanaIfType::voiceEM},
		{"voiceFXO", IanaIfType::voiceFXO},
		{"voiceFXS", IanaIfType::voiceFXS},
		{"voiceEncap", IanaIfType::voiceEncap},
		{"voiceOverIp", IanaIfType::voiceOverIp},
		{"atmDxi", IanaIfType::atmDxi},
		{"atmFuni", IanaIfType::atmFuni},
		{"atmIma", IanaIfType::atmIma},
		{"pppMultilinkBundle", IanaIfType::pppMultilinkBundle},
		{"ipOverCdlc", IanaIfType::ipOverCdlc},
		{"ipOverClaw", IanaIfType::ipOverClaw},
		{"stackToStack", IanaIfType::stackToStack},
		{"virtualIpAddress", IanaIfType::virtualIpAddress},
		{"mpc", IanaIfType::mpc},
		{"ipOverAtm", IanaIfType::ipOverAtm},
		{"iso88025Fiber", IanaIfType::iso88025Fiber},
		{"tdlc", IanaIfType::tdlc},
		{"gigabitEthernet", IanaIfType::gigabitEthernet},
		{"hdlc", IanaIfType::hdlc},
		{"lapf", IanaIfType::lapf},
		{"v37", IanaIfType::v37},
		{"x25mlp", IanaIfType::x25mlp},
		{"x25huntGroup", IanaIfType::x25huntGroup},
		{"transpHdlc", IanaIfType::transpHdlc},
		{"interleave", IanaIfType::interleave},
		{"fast", IanaIfType::fast},
		{"ip", IanaIfType::ip},
		{"docsCableMaclayer", IanaIfType::docsCableMaclayer},
		{"docsCableDownstream", IanaIfType::docsCableDownstream},
		{"docsCableUpstream", IanaIfType::docsCableUpstream},
		{"a12MppSwitch", IanaIfType::a12MppSwitch},
		{"tunnel", IanaIfType::tunnel},
		{"coffee", IanaIfType::coffee},
		{"ces", IanaIfType::ces},
		{"atmSubInterface", IanaIfType::atmSubInterface},
		{"l2vlan", IanaIfType::l2vlan},
		{"l3ipvlan", IanaIfType::l3ipvlan},
		{"l3ipxvlan", IanaIfType::l3ipxvlan},
		{"digitalPowerline", IanaIfType::digitalPowerline},
		{"mediaMailOverIp", IanaIfType::mediaMailOverIp},
		{"dtm", IanaIfType::dtm},
		{"dcn", IanaIfType::dcn},
		{"ipForward", IanaIfType::ipForward},
		{"msdsl", IanaIfType::msdsl},
		{"ieee1394", IanaIfType::ieee1394},
		{"if-gsn", IanaIfType::if_gsn},
		{"dvbRccMacLayer", IanaIfType::dvbRccMacLayer},
		{"dvbRccDownstream", IanaIfType::dvbRccDownstream},
		{"dvbRccUpstream", IanaIfType::dvbRccUpstream},
		{"atmVirtual", IanaIfType::atmVirtual},
		{"mplsTunnel", IanaIfType::mplsTunnel},
		{"srp", IanaIfType::srp},
		{"voiceOverAtm", IanaIfType::voiceOverAtm},
		{"voiceOverFrameRelay", IanaIfType::voiceOverFrameRelay},
		{"idsl", IanaIfType::idsl},
		{"compositeLink", IanaIfType::compositeLink},
		{"ss7SigLink", IanaIfType::ss7SigLink},
		{"propWirelessP2P", IanaIfType::propWirelessP2P},
		{"frForward", IanaIfType::frForward},
		{"rfc1483", IanaIfType::rfc1483},
		{"usb", IanaIfType::usb},
		{"ieee8023adLag", IanaIfType::ieee8023adLag},
		{"bgppolicyaccounting", IanaIfType::bgppolicyaccounting},
		{"frf16MfrBundle", IanaIfType::frf16MfrBundle},
		{"h323Gatekeeper", IanaIfType::h323Gatekeeper},
		{"h323Proxy", IanaIfType::h323Proxy},
		{"mpls", IanaIfType::mpls},
		{"mfSigLink", IanaIfType::mfSigLink},
		{"hdsl2", IanaIfType::hdsl2},
		{"shdsl", IanaIfType::shdsl},
		{"ds1FDL", IanaIfType::ds1FDL},
		{"pos", IanaIfType::pos},
		{"dvbAsiIn", IanaIfType::dvbAsiIn},
		{"dvbAsiOut", IanaIfType::dvbAsiOut},
		{"plc", IanaIfType::plc},
		{"nfas", IanaIfType::nfas},
		{"tr008", IanaIfType::tr008},
		{"gr303RDT", IanaIfType::gr303RDT},
		{"gr303IDT", IanaIfType::gr303IDT},
		{"isup", IanaIfType::isup},
		{"propDocsWirelessMaclayer", IanaIfType::propDocsWirelessMaclayer},
		{"propDocsWirelessDownstream", IanaIfType::propDocsWirelessDownstream},
		{"propDocsWirelessUpstream", IanaIfType::propDocsWirelessUpstream},
		{"hiperlan2", IanaIfType::hiperlan2},
		{"propBWAp2Mp", IanaIfType::propBWAp2Mp},
		{"sonetOverheadChannel", IanaIfType::sonetOverheadChannel},
		{"digitalWrapperOverheadChannel", IanaIfType::digitalWrapperOverheadChannel},
		{"aal2", IanaIfType::aal2},
		{"radioMAC", IanaIfType::radioMAC},
		{"atmRadio", IanaIfType::atmRadio},
		{"imt", IanaIfType::imt},
		{"mvl", IanaIfType::mvl},
		{"reachDSL", IanaIfType::reachDSL},
		{"frDlciEndPt", IanaIfType::frDlciEndPt},
		{"atmVciEndPt", IanaIfType::atmVciEndPt},
		{"opticalChannel", IanaIfType::opticalChannel},
		{"opticalTransport", IanaIfType::opticalTransport},
		{"propAtm", IanaIfType::propAtm},
		{"voiceOverCable", IanaIfType::voiceOverCable},
		{"infiniband", IanaIfType::infiniband},
		{"teLink", IanaIfType::teLink},
		{"q2931", IanaIfType::q2931},
		{"virtualTg", IanaIfType::virtualTg},
		{"sipTg", IanaIfType::sipTg},
		{"sipSig", IanaIfType::sipSig},
		{"docsCableUpstreamChannel", IanaIfType::docsCableUpstreamChannel},
		{"econet", IanaIfType::econet},
		{"pon155", IanaIfType::pon155},
		{"pon622", IanaIfType::pon622},
		{"bridge", IanaIfType::bridge},
		{"linegroup", IanaIfType::linegroup},
		{"voiceEMFGD", IanaIfType::voiceEMFGD},
		{"voiceFGDEANA", IanaIfType::voiceFGDEANA},
		{"voiceDID", IanaIfType::voiceDID},
		{"mpegTransport", IanaIfType::mpegTransport},
		{"sixToFour", IanaIfType::sixToFour},
		{"gtp", IanaIfType::gtp},
		{"pdnEtherLoop1", IanaIfType::pdnEtherLoop1},
		{"pdnEtherLoop2", IanaIfType::pdnEtherLoop2},
		{"opticalChannelGroup", IanaIfType::opticalChannelGroup},
		{"homepna", IanaIfType::homepna},
		{"gfp", IanaIfType::gfp},
		{"ciscoISLvlan", IanaIfType::ciscoISLvlan},
		{"actelisMetaLOOP", IanaIfType::actelisMetaLOOP},
		{"fcipLink", IanaIfType::fcipLink},
		{"rpr", IanaIfType::rpr},
		{"qam", IanaIfType::qam},
		{"lmp", IanaIfType::lmp},
		{"cblVectaStar", IanaIfType::cblVectaStar},
		{"docsCableMCmtsDownstream", IanaIfType::docsCableMCmtsDownstream},
		{"adsl2", IanaIfType::adsl2},
		{"macSecControlledIF", IanaIfType::macSecControlledIF},
		{"macSecUncontrolledIF", IanaIfType::macSecUncontrolledIF},
		{"aviciOpticalEther", IanaIfType::aviciOpticalEther},
		{"atmbond", IanaIfType::atmbond},
		{"voiceFGDOS", IanaIfType::voiceFGDOS},
		{"mocaVersion1", IanaIfType::mocaVersion1},
		{"ieee80216WMAN", IanaIfType::ieee80216WMAN},
		{"adsl2plus", IanaIfType::adsl2plus},
		{"dvbRcsMacLayer", IanaIfType::dvbRcsMacLayer},
		{"dvbTdm", IanaIfType::dvbTdm},
		{"dvbRcsTdma", IanaIfType::dvbRcsTdma},
		{"x86Laps", IanaIfType::x86Laps},
		{"wwanPP", IanaIfType::wwanPP},
		{"wwanPP2", IanaIfType::wwanPP2},
		{"voiceEBS", IanaIfType::voiceEBS},
		{"ifPwType", IanaIfType::ifPwType},
		{"ilan", IanaIfType::ilan},
		{"pip", IanaIfType::pip},
		{"aluELP", IanaIfType::aluELP},
		{"gpon", IanaIfType::gpon},
		{"vdsl2", IanaIfType::vdsl2},
		{"capwapDot11Profile", IanaIfType::capwapDot11Profile},
		{"capwapDot11Bss", IanaIfType::capwapDot11Bss},
		{"capwapWtpVirtualRadio", IanaIfType::capwapWtpVirtualRadio},
		{"bits", IanaIfType::bits},
		{"docsCableUpstreamRfPort", IanaIfType::docsCableUpstreamRfPort},
		{"cableDownstreamRfPort", IanaIfType::cableDownstreamRfPort},
		{"vmwareVirtualNic", IanaIfType::vmwareVirtualNic},
		{"ieee802154", IanaIfType::ieee802154},
		{"otnOdu", IanaIfType::otnOdu},
		{"otnOtu", IanaIfType::otnOtu},
		{"ifVfiType", IanaIfType::ifVfiType},
		{"g9981", IanaIfType::g9981},
		{"g9982", IanaIfType::g9982},
		{"g9983", IanaIfType::g9983},
		{"aluEpon", IanaIfType::aluEpon},
		{"aluEponOnu", IanaIfType::aluEponOnu},
		{"aluEponPhysicalUni", IanaIfType::aluEponPhysicalUni},
		{"aluEponLogicalLink", IanaIfType::aluEponLogicalLink},
		{"aluGponOnu", IanaIfType::aluGponOnu},
		{"aluGponPhysicalUni", IanaIfType::aluGponPhysicalUni},
		{"vmwareNicTeam", IanaIfType::vmwareNicTeam},
		{"docsOfdmDownstream", IanaIfType::docsOfdmDownstream},
		{"docsOfdmaUpstream", IanaIfType::docsOfdmaUpstream},
		{"gfast", IanaIfType::gfast},
		{"sdci", IanaIfType::sdci},
		{"xboxWireless", IanaIfType::xboxWireless},
		{"fastdsl", IanaIfType::fastdsl},
		{"docsCableScte55d1FwdOob", IanaIfType::docsCableScte55d1FwdOob},
		{"docsCableScte55d1RetOob", IanaIfType::docsCableScte55d1RetOob},
		{"docsCableScte55d2DsOob", IanaIfType::docsCableScte55d2DsOob},
		{"docsCableScte55d2UsOob", IanaIfType::docsCableScte55d2UsOob},
		{"docsCableNdf", IanaIfType::docsCableNdf},
		{"docsCableNdr", IanaIfType::docsCableNdr},
		{"ptm", IanaIfType::ptm},
		{"ghn", IanaIfType::ghn},
		{"otnOtsi", IanaIfType::otnOtsi},
		{"otnOtuc", IanaIfType::otnOtuc},
		{"otnOduc", IanaIfType::otnOduc},
		{"otnOtsig", IanaIfType::otnOtsig},
		{"microwaveCarrierTermination", IanaIfType::microwaveCarrierTermination},
		{"microwaveRadioLinkTerminal", IanaIfType::microwaveRadioLinkTerminal},
		{"ieee8021axDrni", IanaIfType::ieee8021axDrni},
		{"ax25", IanaIfType::ax25},
		{"ieee19061nanocom", IanaIfType::ieee19061nanocom},
		{"cpri", IanaIfType::cpri},
		{"omni", IanaIfType::omni},
		{"roe", IanaIfType::roe},
		{"p2pOverLan", IanaIfType::p2pOverLan}
	};

	auto it = m.find(s);
	if (it == m.end()) return IanaIfType::Unknown;
	return it->second;
}

} // namespace yang
