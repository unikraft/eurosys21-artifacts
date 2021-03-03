FROM debian_env as groonga

# INSTALL GROONGA
ENV APP_NAME groonga
ENV GROONGA_VERSION=11.0.0-1

RUN apt update && apt install -y groonga

RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p groonga -o \" -s\" -w 10  " >> run.sh
RUN echo "cat /root/${APP_NAME}_output/${APP_NAME}.json &> out.txt " >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]