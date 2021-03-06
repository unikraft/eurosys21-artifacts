sudo ../aux/dpdk/build/app/testpmd   -m 1024 -n 4 --proc-type=primary  -- -i


# in testpmd
#port stop all
#set fwd rxonly
#port start all
#start
