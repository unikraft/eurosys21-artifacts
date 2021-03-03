FROM debian_env as redis

# INSTALL REDIS
ENV APP_NAME redis-server
RUN apt-get update && \
    apt-get install --no-install-recommends -y redis

RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p redis-server -t /home/testfiles/test_redis.json &> out.txt" >> run.sh
RUN echo "cat /root/${APP_NAME}_output/${APP_NAME}.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]