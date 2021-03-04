sudo ../aux/dpdk/build/app/dpdk-testpmd -d ../aux/dpdk/build/lib/librte_pipeline.so.3 -d ../aux/dpdk/build/drivers/librte_pmd_octeontx.so.1 -d ../aux/dpdk/build/drivers/librte_mempool_octeontx.so.1  -d ../aux/dpdk/build/lib/librte_timer.so.1 -d ../aux/dpdk/build/lib/librte_ethdev.so.12   -m 1024 -n 4 --proc-type=primary --in-memory  -- -i


# in testpmd
#port stop all
#set fwd rxonly
#start
