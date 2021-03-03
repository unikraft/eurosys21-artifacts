FROM debian_env as postgresql

# INSTALL POSTGRESQL
ENV APP_NAME postgresql
ENV PG_VERSION 9.5

RUN apt-get update && apt-get install -y gnupg

RUN sh -c 'echo "deb http://apt.postgresql.org/pub/repos/apt buster-pgdg main" > /etc/apt/sources.list.d/pgdg.list'
RUN wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | apt-key add -
RUN apt-get update && \
    apt-get install -y -q --no-install-recommends \
    postgresql-$PG_VERSION postgresql-client-$PG_VERSION postgresql-contrib-$PG_VERSION && \
    apt-get clean 

WORKDIR /home
RUN cp /etc/postgresql/${PG_VERSION}/main/postgresql.conf /home/postgresql.conf 
RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p /usr/lib/postgresql/${PG_VERSION}/bin/postgres -o \" -c config_file=config\" -t /home/testfiles/test_postgres.json &> /dev/null" >> run.sh
RUN echo "cat /var/lib/${APP_NAME}/postgres_output/postgres.json" >> run.sh
RUN chmod +x run.sh

USER postgres
RUN    /etc/init.d/postgresql start &&\
    psql --command "CREATE USER docker WITH SUPERUSER PASSWORD 'docker';" &&\
    createdb -O docker docker

RUN echo "host all  all    0.0.0.0/0  md5" >> /etc/postgresql/${PG_VERSION}/main/pg_hba.conf
RUN echo "listen_addresses='*'" >> /etc/postgresql/${PG_VERSION}/main/postgresql.conf

VOLUME  ["/etc/postgresql", "/var/log/postgresql", "/var/lib/postgresql"]

CMD ["./run.sh"]
