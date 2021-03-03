FROM debian_env as lighttpd

# INSTALL LIGHTTPD
ENV APP_NAME lighttpd
RUN apt-get update && \
    apt-get install --no-install-recommends -y ${APP_NAME}

RUN cp /etc/lighttpd/lighttpd.conf /home/conf
WORKDIR /home/
RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p /usr/sbin/lighttpd -o \" -f conf\" -t /home/testfiles/test_http.json &> out.txt " >> run.sh
RUN echo "cat /root/${APP_NAME}_output/${APP_NAME}.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]