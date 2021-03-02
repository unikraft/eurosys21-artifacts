FROM debian_env as exim

# INSTALL EXIM
ENV APP_NAME exim
RUN apt-get update && apt-get install -y exim4-daemon-light

RUN cp /etc/aliases /etc/aliases.stub

WORKDIR /home/
RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p exim4 -o \" -bd -q30m\" -t /home/testfiles/text_exim.json &> out.txt" >> run.sh
RUN echo "cat /root/exim4_output/exim4.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]