FROM debian_env as mysql

# INSTALL mysql
ENV APP_NAME mysql
ENV MYSQL_USER=mysql \
    MYSQL_DATA_DIR=/var/lib/mysql \
    MYSQL_RUN_DIR=/run/mysqld \
    MYSQL_LOG_DIR=/var/log/mysql

RUN wget http://repo.mysql.com/mysql-apt-config_0.8.13-1_all.deb
RUN apt install -y ./mysql-apt-config_0.8.13-1_all.deb
RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y mysql-server \
 && rm -rf ${MYSQL_DATA_DIR} \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /home/
RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p /usr/sbin/mysqld -t /home/testfiles/test_mysql.json &> out.txt " >> run.sh
RUN echo "cat /root/mysqld_output/mysqld.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]