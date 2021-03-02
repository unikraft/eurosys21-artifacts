FROM debian_env as h2o

# INSTALL H20
ENV APP_NAME h2o
RUN apt-get update && \
    apt-get install --no-install-recommends -y ${APP_NAME}

RUN	rm -rf /var/lib/apt/lists/*

RUN	mkdir -p /etc/h2o /var/run/h2o /var/log/h2o
RUN	touch /var/run/h2o/access-log /var/run/h2o/error-log

RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p /usr/bin/h2o -t /home/testfiles/test_http.json &> out.txt" >> run.sh
RUN echo "cat /root/${APP_NAME}_output/${APP_NAME}.json" >> run.sh
RUN chmod +x run.sh

COPY ./h2o.conf /etc/h2o

CMD ["./run.sh"]