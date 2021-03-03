FROM debian_env as apache

# INSTALL APACHE
ENV APP_NAME apache
RUN apt-get update && \
    apt-get install --no-install-recommends -y libpcre2-dev libpcre3-dev libapr1-dev libaprutil1-dev apache2

RUN wget https://downloads.apache.org//httpd/httpd-2.4.46.tar.gz
RUN tar xf httpd-2.4.46.tar.gz
WORKDIR /home/httpd-2.4.46
RUN ./configure && make
RUN cp httpd /home/httpd_bin

WORKDIR /home
RUN mkdir -p /usr/local/apache2/
RUN	mkdir -p /home/httpd/htdocs /home/httpd/htdocs/logs
RUN mkdir -p /usr/local/apache2/logs/
RUN touch /home/httpd/htdocs/logs/error_log

RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p httpd_bin -o \" -f /usr/local/apache2/conf -X -k start\" -w 10 &> out.txt" >> run.sh
RUN echo "cat /root/httpd_bin_output/httpd_bin.json" >> run.sh
RUN chmod +x run.sh

COPY ./index.html /home/httpd/htdocs/
COPY ./apache.conf /usr/local/apache2/conf

CMD ["./run.sh"]