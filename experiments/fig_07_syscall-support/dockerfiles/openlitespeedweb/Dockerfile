FROM debian_env as openlitespeed

# INSTALL OPENLIGHTSPEED
ENV APP_NAME openlitespeed
RUN apt-get update

RUN wget https://openlitespeed.org/packages/openlitespeed-1.7.8.tgz
RUN tar -zxvf openlitespeed-*.tgz

WORKDIR /home/openlitespeed/
RUN ./install.sh
RUN cp /home/openlitespeed/bin/openlitespeed /usr/bin/openlitespeed
WORKDIR /home/
RUN sed -i 's/80/8088/' testfiles/test_http.json
RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p /usr/bin/openlitespeed -t /home/testfiles/test_http.json &> out.txt" >> run.sh
RUN echo "cat /root/${APP_NAME}_output/${APP_NAME}.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]