FROM debian_env as nginx

# INSTALL NGINX
ENV APP_NAME nginx
RUN apt-get update && \
    apt-get install --no-install-recommends -y nginx && \
    apt-get clean && \
    echo "\ndaemon off;" >> /etc/nginx/nginx.conf && \
    rm -rf /var/lib/apt/lists/*

VOLUME ["/var/www", "/etc/nginx/sites-enabled", "/etc/nginx/certs", "/etc/nginx/conf.d"]
EXPOSE 80 443

RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p /usr/sbin/nginx -t /home/testfiles/test_http.json &> out.txt" >> run.sh
RUN echo "cat /root/${APP_NAME}_output/${APP_NAME}.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]