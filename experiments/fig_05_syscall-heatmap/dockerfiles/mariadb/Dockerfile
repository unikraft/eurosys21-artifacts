FROM debian_env as mariadb

# INSTALL MARIADB
ENV APP_NAME mariadb
RUN apt-get update && \
    apt-get install --no-install-recommends -y build-essential bison mariadb-server && \
    sed -i 's/^\(bind-address\s.*\)/# \1/' /etc/mysql/my.cnf && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* && \
    update-rc.d -f mysql disable

WORKDIR /home/
RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p mysqld -t /home/testfiles/test_mysql.json &> out.txt " >> run.sh
RUN echo "cat /root/mysqld_output/mysqld.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]