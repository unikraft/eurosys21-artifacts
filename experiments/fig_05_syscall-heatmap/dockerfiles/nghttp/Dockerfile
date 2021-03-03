FROM debian_env as nghttp

# INSTALL NGHTTP
ENV APP_NAME nghttp
RUN apt-get update && \
    apt-get install --no-install-recommends -y ${APP_NAME}2

WORKDIR /home/
RUN sed -i 's/80/3000/' testfiles/test_http.json
RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p nghttpx -t /home/testfiles/test_http.json &> out.txt" >> run.sh
RUN echo "cat /root/${APP_NAME}x_output/${APP_NAME}x.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]