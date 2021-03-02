FROM debian_env as tntnet

# INSTALL TNTNET
ENV APP_NAME tntnet
RUN apt-get update && \
    apt-get install --no-install-recommends -y ${APP_NAME}

WORKDIR /home/
RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p tntnet -t /home/testfiles/test_http.json &> out.txt" >> run.sh
RUN echo "cat /root/${APP_NAME}_output/${APP_NAME}.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]