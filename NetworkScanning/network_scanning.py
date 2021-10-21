from scapy.all import *

"""
Use scapy to implement the SYNScan and DNSScan
"""
ports = [25,80,53,443,445,8080,8443]

def SynScan(host):
    """
    Send a SYN packet and wait for ACK from destination ports.
    """
    answers,unanswers = sr(IP(dst=host)/TCP(dport=ports,flags="S"),timeout=2,verbose=0)
    print("Open ports at %s:" % host)
    for (sent_packet,received_packet,) in answers:
        if sent_packet[TCP].dport == received_packet[TCP].sport:
            print(sent_packet[TCP].dport)

def DNSScan(host):
    """
    Send a DNS request and wait for response to verify if there are
    DNS servers waiting at destination ports.
    """
    answers,unanswers = sr(IP(dst=host)/UDP(dport=53)/DNS(rd=1,qd=DNSQR(qname="google.com")),timeout=2,verbose=0)
    if answers:
        print("DNS Server at %s"%host)
    
host = "8.8.8.8"

SynScan(host)
DNSScan(host)