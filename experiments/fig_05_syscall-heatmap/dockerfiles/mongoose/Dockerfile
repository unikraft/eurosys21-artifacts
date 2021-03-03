FROM debian_env as mongoose

# INSTALL MONGOOSE
ENV APP_NAME mongoose_linux
RUN apt-get update && apt-get install -y procps
RUN curl -O https://vcon.io/downloads/mws/mongoose_linux
RUN chmod +x mongoose_linux

WORKDIR /home/
#RUN sleep 30 && kill -9 $(ps -ef | grep "mongoose_linux" | awk '{print $2}') &
RUN sed -i 's/80/8000/' testfiles/test_http.json
RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p mongoose_linux -t /home/testfiles/test_http.json &> out.txt " >> run.sh
RUN echo "cat /root/${APP_NAME}_output/${APP_NAME}.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]