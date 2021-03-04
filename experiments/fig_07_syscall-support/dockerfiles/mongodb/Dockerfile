FROM debian_env as mongo_db

# INSTALL MONGODB
ENV APP_NAME mongod
RUN apt-get update && apt-get install -y gnupg wget
RUN wget -qO - https://www.mongodb.org/static/pgp/server-4.4.asc | apt-key add -
RUN echo "deb http://repo.mongodb.org/apt/debian buster/mongodb-org/4.4 main" | tee /etc/apt/sources.list.d/mongodb-org-4.4.list
RUN apt-get update && \
    apt-get install -y mongodb-org && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

VOLUME ["/data/db"]

RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p /usr/bin/mongod -t /home/testfiles/mongodb.json &> out.txt" >> run.sh
RUN echo "cat /root/${APP_NAME}_output/${APP_NAME}.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]