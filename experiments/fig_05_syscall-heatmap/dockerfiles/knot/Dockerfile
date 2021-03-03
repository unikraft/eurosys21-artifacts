FROM debian_env as knot

# INSTALL KNOT
ENV APP_NAME knot
RUN apt-get update && apt-get install -y knot

WORKDIR /home/
RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p knotd -t /home/testfiles/text_knot.json &> out.txt" >> run.sh
RUN echo "cat /root/knotd_output/knotd.json" >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]