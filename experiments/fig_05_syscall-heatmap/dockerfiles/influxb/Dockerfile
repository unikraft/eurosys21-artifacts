FROM debian_env as influxdb

# INSTALL INFLUXDB
ENV APP_NAME influxd
RUN apt-get update && \
    apt-get install --no-install-recommends -y ${APP_NAME}b

WORKDIR /home/
RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p influxd -t /home/testfiles/test_influx.json &> out.txt" >> run.sh
RUN echo "cat /root/${APP_NAME}_output/${APP_NAME}.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]