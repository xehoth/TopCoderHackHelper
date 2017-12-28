/*******************************************************************************
 * Copyright (c) 2017, xehoth
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/
#ifndef ETHERHEADER_H
#define ETHERHEADER_H
#include <arpa/inet.h>
#include <pcap.h>
#include <sys/stat.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>

const int ETH_ALEN = 6;

struct EtherHeader {
    u_int8_t dstMac[ETH_ALEN];  // 目标mac地址
    u_int8_t srcMac[ETH_ALEN];  // 源mac地址
    uint16_t etherType;         // 以太网类型
    // packet tyoe id field
    // 0800 ip
    // 0806 arp
    // 86dd ipv6
};

struct IpHeader {
    u_int8_t versionIhl;  // 版本 + 首部长度
    u_int8_t tos;         // 服务类型
    u_int16_t totLen;     // 总长度
    u_int16_t id;         // 标志
    u_int16_t fragOff;    // 分片偏移
    u_int8_t ttl;         // 生存时间
    u_int8_t protocol;    // 协议
    u_int16_t checkSum;   // 检验和
    u_int32_t srcAddr;    // 源IP地址
    u_int32_t dstAddr;    // 目的IP地址
};

struct TcpHeader {
    u_int16_t srcPort;  // 源端口号
    u_int16_t dstPort;  // 目的端口号
    u_int32_t seq;      // 序列号
    u_int32_t ack;      // 确认号
    u_int8_t off;       // data offset
    u_int8_t flags;
    u_int16_t win;  // 16 位窗口大小
    u_int16_t sum;  // 16 位TCP检验和
    u_int16_t urp;  // 16 位紧急指针
};

inline int getOffset(u_char offset) { return (offset >> 2); }

class PcapListener {
   public:
    PcapListener() { init(); }
    PcapListener(const char *filter) {
        setFilter(filter);
        init();
    }

    void init() {
        devs = nullptr;
        initFlag = false;
        handle = nullptr;

        memset(errbuf, 0, sizeof(errbuf));
        int ret = pcap_findalldevs(&devs, errbuf);
        if (ret == -1 || devs == 0) {
            perror("pcap_findalldevs");
            initFlag = false;
            return;
        }

        int elistIndex = 0;
        for (pcap_if *d = devs; d != nullptr; d = d->next) {
            std::cout << ++elistIndex << ". " << d->name;
            if (d->description) {
                std::cout << "\n\t(" << d->description << ")" << std::endl;
            } else {
                std::cout << " (No description available)\n";
            }
        }

        std::cout << "Please select the interface you are using:" << std::endl;

        int chooseIndex;
        std::cin >> chooseIndex;
        char interface_name[100];

        if (chooseIndex <= elistIndex) {
            int tmp = 1;
            pcap_if *d = devs;
            for (d = devs; d != nullptr && tmp < chooseIndex; d = d->next) {
                tmp++;
            }
            if (d != nullptr) {
                strcpy(interface_name, d->name);
            } else {
                std::cout << "The interface you have selected is invalid!"
                          << std::endl;
                interface_name[0] = 0;
                initFlag = false;
                return;
            }
        }
        std::cout << "You have selected " << interface_name << std::endl;

        handle = pcap_open_live(interface_name, 65535, 0 /*no promisc */, 20,
                                errbuf);
        if (handle == nullptr) {
            std::cout << "pcap_open_live failed." << std::endl;
            initFlag = false;
            return;
        } else {
            std::cout << "OPEN DEVICE SUCCEED" << std::endl;
            initFlag = true;
        }

        bpf_program fp;
        bpf_u_int32 maskp, netp;
        ret = pcap_lookupnet(interface_name, &netp, &maskp, errbuf);

        if (ret == -1) {
            perror("pcap_lookupnet");
            initFlag = false;
            return;
        }

        if (pcap_compile(handle, &fp, filter, 0, netp) == -1) {
            perror("pcap_compile");
            initFlag = false;
            return;
        }
        if (pcap_setfilter(handle, &fp) == -1) {
            perror("pcap_setfilter");
            initFlag = false;
            return;
        }

        std::cout << "Initializing....\nEVERYTHING IS OK. START WORKING ... "
                  << std::endl;
    }

    template <typename T>
    void runLoop(T func) {
        unsigned int oldseq = -1;
        int lastLen = -1;
        for (;;) {
            pcap_pkthdr hdr;
            const u_char *packet;
            packet = pcap_next(handle, &hdr);
            if (hdr.len < 14 + 20 + 20 || hdr.len > 65535 || packet == 0)
                continue;
            EtherHeader *ether = (EtherHeader *)packet;
            if (ether->etherType == 0x0008) {
                IpHeader *iphdr = (IpHeader *)(packet + 14);
                TcpHeader *tcphdr = 0;
                if (iphdr->protocol == IPPROTO_TCP) {
                    tcphdr = (TcpHeader *)(packet + 14 + 20);
                    if (tcphdr->srcPort == htons(5001)) {
                        if (oldseq != -1 && ntohl(tcphdr->seq) <= oldseq) {
                            continue;
                        }
                    }
                } else {
                    continue;
                }
                oldseq = ntohl(tcphdr->seq);
                packet = (u_char *)(packet + 14 + 20 + getOffset(tcphdr->off));
                int n = hdr.len - 14 - 20 - getOffset(tcphdr->off);
                std::cout << "begin analyzing packet ... \n" << std::endl;
                lastLen = func(packet, n, lastLen);
                if (lastLen == 0) lastLen = func(packet, n, lastLen);
                oldseq = ntohl(tcphdr->seq);
            } else {
                printf("etherType = %x\n", ether->etherType);
            }
        }
    }

    void setFilter(const char *filter) { this->filter = filter; }

    bool getInitFlag() { return initFlag; }

   private:
    pcap_if *devs;
    char errbuf[1024];
    bool initFlag;
    pcap_t *handle;
    const char *filter;
};
#endif