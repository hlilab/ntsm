#!/usr/bin/env -S make -Rrf
ifdef profile
SHELL=/usr/bin/time -f '=> ntsmSiteGen: %e %C' /bin/bash -o pipefail
else
SHELL=/bin/bash -o pipefail
endif

ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

#------------------------------------------------------------
# params
#------------------------------------------------------------

#parameters for generate-sites

w=31 #window size to consider sequences in this region
k=19 #kmer size used in window region
t=4 #threads for any subprocess or tools

#parameters of generate-pca-rot-mat

dims=20

#------------------------------------------------------------
# meta rules
#------------------------------------------------------------

.PRECIOUS: %.sam %.ann %.amb %.bwt %.pac %.sa
.DELETE_ON_ERROR:
.PHONY: check-params generate-sites generate-pca-rot-mat check-param-pca test

generate-sites: check-params $(name)_n0.fa

generate-pca-rot-mat: check-param-pca $(name)_rotationalMatrix.tsv

#check params for generate-sites
check-name-param:
ifndef name
	$(error missing required param 'name' (output file prefix))
endif
check-ref-param: check-name-param
ifndef ref
	$(error missing required param 'ref' (FASTA reference file))
endif
check-params: check-ref-param
ifndef vcf
	$(error missing required param 'vcf' (vcf file containing variants file))
endif

#check params for generate-pca-rot-mat
check-sites-param-pca:
ifndef sites
	$(error missing required param 'sites' (number of missing k-mers allowed per site))
endif
check-param-pca: check-sites-param-pca
ifndef multivcf
	$(error missing required param 'multivcf' (mutlivcf file containing genotypes of various samples))
endif

test:
	python $(ROOT_DIR)/convertTSVtoPCA.py --help
	python $(ROOT_DIR)/extractSNPsfromVCF.py --help

#------------------------------------------------------------
# pipeline rules
#------------------------------------------------------------

$(name)_reference.fa: $(ref)
	ln -s $< $@
	
$(name)_snps.vcf: $(vcf)
	ln -s $< $@
	
$(name)_multi.vcf: $(multivcf)
	ln -s $< $@

#link index files if they already exist
ifneq ("$(wildcard $(ref).ann)","")
$(name)_reference.fa.ann: $(ref).ann
	ln -s $< $@ -f
$(name)_reference.fa.amb: $(ref).amb
	ln -s $< $@ -f
$(name)_reference.fa.bwt: $(ref).bwt
	ln -s $< $@ -f
$(name)_reference.fa.pac: $(ref).pac
	ln -s $< $@ -f
$(name)_reference.fa.sa: $(ref).sa
	ln -s $< $@ -f
else
#index reference file
%.fa.ann %.fa.amb %.fa.bwt %.fa.pac %.fa.sa: %.fa
	bwa index $<
endif

#create snp sub k-mer fasta files
%_subKmers.fa: %_snps.vcf %_reference.fa
	python $(ROOT_DIR)/extractSNPsfromVCF.py -v $< -f $*_reference.fa -s $(k) -k $(w) > $@

#check for k-mer uniqueness via alignment
%.sai: %_subKmers.fa %_reference.fa %_reference.fa.ann %_reference.fa.amb %_reference.fa.bwt %_reference.fa.pac %_reference.fa.sa
	bwa aln -t $(t) -n 1 $(name)_reference.fa $< > $@

%.sam: %.sai %_subKmers.fa %_reference.fa
	bwa samse $(name)_reference.fa $< $(name)_subKmers.fa > $@

#filter repetitive sub k-mers & create concatenated sub k-mer fasta files
$(name)_n0.fa: $(name).sam
	perl $(ROOT_DIR)/filterRepetiveSNP.pl <(samtools view $<) $(name) $(w) $(k)
	
$(name)_matrix.tsv: $(sites) $(name)_reference.fa $(name)_multi.vcf
	ntsmVCF -k $(k) -t $(t) -w $(w) -p $(name) -s $< -r $(name)_reference.fa $(name)_multi.vcf

$(name)_rotationalMatrix.tsv: $(name)_matrix.tsv
	python $(ROOT_DIR)/convertTSVtoPCA.py -p $(name) -m $< -n $(dims)

