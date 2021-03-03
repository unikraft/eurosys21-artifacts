FROM debian_env as nullmailer

# INSTALL NULLMAILER
ENV APP_NAME nullmailer
RUN apt-get update && \
    apt-get install --no-install-recommends -y ${APP_NAME}

WORKDIR /home/
RUN cp /usr/sbin/nullmailer-send /home/nullmailer

RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p nullmailer -w 10 &> out.txt " >> run.sh
RUN echo "cat /root/${APP_NAME}_output/${APP_NAME}.json  " >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]
