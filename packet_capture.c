/*
****************************************************************************
 *
 * Example compiler command-line for GCC:
 *   gcc -lpcap packet_capture.c -o sniffer
 *
 *   run using ./sniffer ip
 *
 ****************************************************************************
 */

#include <pcap.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>

void pcap_fatal(const char *failed_in, const char *errbuf) {
     printf("Fatal Error in %s: %s\n", failed_in, errbuf);
     exit(1);
}

void my_callback(u_char *arg, const struct pcap_pkthdr* pkthdr,
        const u_char* packet)
{
    int i=0;
    static int count=0;

    printf("Packet Count: %d\n", ++count);    /* Number of Packets */
    printf("Received Packet Size: %d\n", pkthdr->len);    /* Length of header */
    printf("Received Capture length: %d\n", pkthdr->caplen);    /* Length of portion present */
    printf("Comment: %d\n", pkthdr->comment);    /* comment */
    printf("Recieved time value : %d\n", pkthdr->ts);    /* Time stamp */
    /*
    printf("Payload:\n");                     // And now the data
    for(i=0;i<pkthdr->len;i++) {
        if(isprint(packet[i]))                // Check if the packet data is printable
            printf("%c ",packet[i]);          // Print it
        else
            printf(" . ",packet[i]);          // If not print a .
        if((i%16==0 && i!=0) || i==pkthdr->len-1)
            printf("\n");
    }
    */
}

int main(int argc,char **argv)
{
    int i;
    char *dev;
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* descr;
    const u_char *packet;
    struct pcap_pkthdr hdr;
    struct ether_header *eptr;    /* net/ethernet.h */
    struct bpf_program fp;        /* hold compiled program */
    bpf_u_int32 maskp;            /* subnet mask */
    bpf_u_int32 netp;             /* ip */
    u_int16_t       ip_sum;          /* checksum                  */
    u_int8_t        ip_ttl;          /* time to live              */

    if(argc != 2){
        fprintf(stdout, "Usage: %s \"expression\"\n"
            ,argv[0]);
        return 0;
    }

    /* Now get a device */
    dev = pcap_lookupdev(errbuf);

    if(dev == NULL) {
        fprintf(stderr, "%s\n", errbuf);
        exit(1);
    }
        /* Get the network address and mask */
    pcap_lookupnet(dev, &netp, &maskp, errbuf);

    /* open device for reading in promiscuous mode */
    descr = pcap_open_live(dev, BUFSIZ, 0,1, errbuf);
    if(descr == NULL) {
        printf("pcap_open_live(): %s\n", errbuf);
        exit(1);
    }

    /* Now we'll compile the filter expression*/
    if(pcap_compile(descr, &fp, argv[1], 0, netp) == -1) {
        fprintf(stderr, "Error calling pcap_compile\n");
        exit(1);
    }

    /* set the filter */
    if(pcap_setfilter(descr, &fp) == -1) {
        fprintf(stderr, "Error setting filter\n");
        exit(1);
    }

    /* loop for callback function */
    pcap_loop(descr, -1, my_callback, NULL);
    return 0;
}
