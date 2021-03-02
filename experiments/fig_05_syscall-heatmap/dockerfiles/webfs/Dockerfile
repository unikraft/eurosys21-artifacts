FROM debian_env as webfs

# INSTALL WEBFS
ENV APP_NAME webfs
RUN apt-get update && \
    apt-get install --no-install-recommends -y ${APP_NAME}

RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p /usr/bin/webfsd -o \" -p 80 \" -t /home/testfiles/test_http.json &> out.txt" >> run.sh
RUN echo "cat /root/webfsd_output/webfsd.json" >> run.sh
RUN chmod +x run.sh

COPY ./mime.types /etc/mime.types

CMD ["./run.sh"]