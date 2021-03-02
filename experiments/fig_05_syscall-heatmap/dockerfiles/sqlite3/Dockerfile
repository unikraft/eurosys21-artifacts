FROM debian_env as sqlite3

# INSTALL SQLITE3
ENV APP_NAME sqlite3
RUN apt-get update && \
    apt-get install --no-install-recommends -y ${APP_NAME}

RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p ${APP_NAME} -t /home/testfiles/test_sqlite.json &> out.txt" >> run.sh
RUN echo "cat /root/${APP_NAME}_output/${APP_NAME}.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]