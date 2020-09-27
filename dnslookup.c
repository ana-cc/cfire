#include <stdio.h>
#include <ldns/ldns.h>


void
retrieve(char results[][100], ldns_rr_list *a ){
        int k = ldns_rr_list_rr_count(a);
     	size_t i;
        for (i = 0; i < ldns_rr_list_rr_count(a); i++) {
                ldns_rr *r;
        	r = ldns_rr_list_rr(a, i);
 		size_t j;
		for (j = 0; j < ldns_rr_rd_count(r); j++) {
                	ldns_rdf *rdf = ldns_rr_rdf(r, j);
			ldns_buffer *buf = ldns_buffer_new(40);
			ldns_rdf2buffer_str(buf, rdf);
			//printf("%s\n", buf->_data);
                        strcpy(results[i], buf->_data);
			}
                }
	
}
	

int
main(int argc, char **argv) {

	if (argc < 2) {
		printf("No domain provided\n");
		return 1;
	}
          
	int aflag, aaaaflag, nsflag, mxflag, ch, num;
        int flags = 0;
       
	while ((ch = getopt(argc, argv, "t:")) != -1) {
		switch (ch) {		/* Indent the switch. */
		case 't':		/* Don't indent the case. */
			if (strcmp(optarg, "A") == 0) {
				aflag = 1;
				flags = 1;
			} else if (strcmp(optarg, "AAAA") == 0) {
				aaaaflag = 1;
				flags = 1;
			} else if (strcmp(optarg, "MX") == 0) {
				mxflag = 1;
				flags = 1;
			} else if (strcmp(optarg, "NS") == 0) {
				nsflag = 1;
				flags = 1;
			} else {
				printf("Option not recognized\n");
			}
			break;
		default:
			printf("Requesting an A record by default\n");
			aflag = 1;
		}
	}
        if (flags == 0){
	    aflag = 1;
	}

	size_t index;

	ldns_rdf *domain;
        ldns_resolver *res;
        ldns_status status;
        ldns_pkt *packet;
        ldns_rr_list *a;

	domain = ldns_dname_new_frm_str(argv[optind]);
        status = ldns_resolver_new_frm_file(&res, NULL);
        if (status == 1){
		printf("Failed to initialise DNS resolver\n");
	}
	
	enum ldns_enum_rr_type type;

        if (aflag == 1) {
           type = LDNS_RR_TYPE_A;
        } else if (aaaaflag ==1) {
           type = LDNS_RR_TYPE_AAAA;
        } else if (nsflag ==1) {
           type = LDNS_RR_TYPE_NS;
        } else if (mxflag ==1) {
           type = LDNS_RR_TYPE_MX;
        }

        packet = ldns_resolver_search(res,
                                 domain,
                                 type,
                                 LDNS_RR_CLASS_IN,
                                 LDNS_RD);

	a = ldns_pkt_rr_list_by_type(packet,
                                     type,
                                     LDNS_SECTION_ANSWER);
	ldns_rr_list_sort(a); 

        int k = ldns_rr_list_rr_count(a);
        char results[k][100];
	retrieve(results, a); 

        if ((nsflag == 1) || (mxflag == 1)){
	        for (int i = 0; i < k; i++) {
                        domain = ldns_dname_new_frm_str(results[i]);
	    	        packet = ldns_resolver_search(res,
                                     domain,
                                     LDNS_RR_TYPE_A,
                                     LDNS_RR_CLASS_IN,
                                     LDNS_RD);
    
	    		a = ldns_pkt_rr_list_by_type(packet,
                                         LDNS_RR_TYPE_A,
                                         LDNS_SECTION_ANSWER);
        		int k = ldns_rr_list_rr_count(a);
             		char resultsns[k][100];
	                retrieve(resultsns, a); 

	        	for (int i = 0; i < k; i++) {
				puts(resultsns[i]);
			}

                }
	        for (int i = 0; i < k; i++) {
                        domain = ldns_dname_new_frm_str(results[i]);
	    	        packet = ldns_resolver_search(res,
                                     domain,
                                     LDNS_RR_TYPE_AAAA,
                                     LDNS_RR_CLASS_IN,
                                     LDNS_RD);
    
	    		a = ldns_pkt_rr_list_by_type(packet,
                                         LDNS_RR_TYPE_AAAA,
                                         LDNS_SECTION_ANSWER);
        		int k = ldns_rr_list_rr_count(a);
             		char resultsns[k][100];
	                retrieve(resultsns, a); 

	        	for (int i = 0; i < k; i++) {
				puts(resultsns[i]);
			}
		}
        } else {
	        for (int i = 0; i < k; i++) {
			puts(results[i]);
		}
	}


        ldns_rr_list_deep_free(a);
        ldns_pkt_free(packet);
        ldns_resolver_deep_free(res);
}
