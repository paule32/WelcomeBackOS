
# Master-Kontextdatei – Kernel Networking & QEMU Projekt

Diese Datei fasst alle bisherigen technischen Inhalte, Diagramme, Erklärungen und Architekturentscheidungen zusammen, sodass zukünftige Sessions mit ChatGPT sofort wieder vollständig im Thema sind.

---

## 1. Gesamtüberblick des Projekts

Ziel: Aufbau eines eigenen Kernel-Netzwerkstacks (Ethernet → ARP → IPv4 → UDP/TCP) inklusive QEMU‑Netzwerkemulation und zugehöriger Dokumentationsgrafiken.

Das Projekt umfasst:

- Aufbau eines minimalen, aber funktionalen Netzwerkstacks im eigenen Kernel  
- QEMU-Netzwerkkonfiguration mit `-netdev` und `-device`  
- Erstellung von technischen Diagrammen (2D/3D, RFC‑Style, Flows, Header‑Darstellungen)  
- Erstellung einer Dokumentationsbasis (README.md, Grafiken, Übersichten)  

---

## 2. QEMU Netzwerk-Konfiguration

### Grundstruktur:
- QEMU trennt Backend (`-netdev`) und Netzwerkgerät (`-device`).
- Beispiel für E1000 (empfohlen für Kernel-Prototypen):

```
-netdev user,id=n0
-device e1000,netdev=n0,mac=52:54:00:12:34:56
```

### Beispiele:
- Portweiterleitung:
```
-netdev user,id=n0,hostfwd=tcp::1234-:1234
```

- Virtio statt E1000:
```
-device virtio-net-pci,netdev=n0
```

---

## 3. Netzwerkstack im Kernel

### Schichtenmodell:

1. NIC-Treiber (z. B. E1000)
2. Ethernet-Schicht  
3. ARP  
4. IPv4  
5. Transport-Protokolle (UDP, ICMP, später TCP)
6. Socket-ähnliche API (optional)
7. Anwendungen

### Anforderungen:
- PCI-Scan zur NIC-Erkennung  
- RX/TX-Ringbuffer  
- IRQ-basierte oder Polling-basierte Paketverarbeitung  

---

## 4. Protokollübersicht

### Ethernet:
- Enthält Destination MAC, Source MAC, EtherType, Payload, CRC.

### ARP:
- Dient der Auflösung IP → MAC  
- ARP Request/Reply Mechanismus

### IPv4:
- Headerfelder: Version, IHL, DSCP/ECN, Length, Identification, Flags, TTL, Protocol, Checksum, Source/Destination IP.

### UDP:
- Source Port, Destination Port, Length, Checksum

### TCP:
- Source Port, Destination Port  
- Sequence/Ack Number  
- Flags (SYN, ACK, FIN…)  
- Window Size, Checksum  
- TCP-State-Machine nach RFC 793  

---

## 5. Diagramme (erzeugt)

### 3D-Diagramme:
- 3D TCP Header  
- 3D UDP Header  
- 3D Ethernet‑Frame‑Flow  
- 3D Routing‑Pfad  
- OSI‑7‑Layer 3D Modell  
- Endgerät → DNS → DHCP → Router → WAN Übersicht  

### RFC-Style Diagramme:
- TCP State Machine (RFC Style)  
- DHCP Flow (RFC 2131)  
- DNS Flow (RFC 1034/1035)  
- Routing Flow (RFC 1812)  
- Ethernet Frame  
- ARP Packet  
- IPv4 Header  
- TCP Header  
- UDP Header  

---

## 6. Architekturfluss End‑zu‑Ende

1. Gerät sendet ARP Request → erhält MAC  
2. IPv4 Header wird aufgebaut  
3. UDP oder TCP baut Transportheader  
4. Ethernet Frame wird gebaut  
5. NIC übergibt Paket an QEMU  
6. QEMU leitet über User‑Backend, Tap oder virtio weiter  
7. Bei DNS‑Queries: Resolver → Root → TLD → Authoritative  
8. Bei DHCP: Discover → Offer → Request → ACK  
9. Router bestimmt nächste Hop-Adresse  

---

## 7. Implementierungs-Notizen

- `net_init()` sollte NIC‑Init, ARP‑Init, IPv4‑Init, UDP/TCP‑Init kombinieren.
- Polling für frühe Prototypen ok; später IRQ‑basiert empfohlen.
- Debugging über QEMU möglich:
```
-d net
```

---

## 8. Empfehlungen für weitere Entwicklung

- Aufbau einer kleinen Socket‑API  
- Implementierung eines ARP‑Caches  
- Einfaches UDP‑Echo zur Funktionsprüfung  
- ICMP Ping zur Basisdiagnose  
- Danach TCP‑Handshake (SYN → SYN‑ACK → ACK)  

---

## 9. Verwendung dieser Kontextdatei in neuen Sessions

Diese Datei kann in **jeder zukünftigen ChatGPT‑Session** hochgeladen werden.  
ChatGPT erkennt diese Datei als vollständigen Kontext und kann sofort weiterarbeiten, ohne dass der gesamte Verlauf erneut eingegeben werden muss.

---

*(Ende der Master-Kontextdatei)*
