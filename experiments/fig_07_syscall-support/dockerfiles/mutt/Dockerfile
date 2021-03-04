FROM debian_env as mutt

# INSTALL MUTT
ENV APP_NAME mutt
RUN apt-get update && \
    apt-get install --no-install-recommends -y ${APP_NAME}

WORKDIR /home/

RUN mkdir -p /root/Mail
RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p mutt -w 10&> out.txt " >> run.sh
RUN echo "cat /root/${APP_NAME}_output/${APP_NAME}.json " >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]
