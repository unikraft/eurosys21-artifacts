FROM debian_env as dovecot

ENV APP_NAME dovecot
RUN apt-get update && apt-get install ca-certificates

# Disable Dovecot TLS during installation to prevent key from being pregenerated
RUN mkdir -p /etc/dovecot && echo "ssl = no" > /etc/dovecot/local.conf

# Install all alpine dovecot packages (except documentation and development files)
RUN apt-get install -y dovecot-core \
    dovecot-sieve dovecot-solr dovecot-antispam \
	dovecot-gssapi \
	dovecot-ldap \
	dovecot-lmtpd \
	dovecot-mysql \
	dovecot-pgsql \
	dovecot-pop3d \
	dovecot-sqlite \
	dovecot-submissiond

# Set logging to STDOUT/STDERR
RUN sed -i -e 's,#log_path = syslog,log_path = /dev/stderr,' \
           -e 's,#info_log_path =,info_log_path = /dev/stdout,' \
           -e 's,#debug_log_path =,debug_log_path = /dev/stdout,' \
	/etc/dovecot/conf.d/10-logging.conf
# Set default passdb to passwd and create the referenced 'users' file
RUN sed -i -e 's,!include auth-system.conf.ext,!include auth-passwdfile.conf.ext,' \
           -e 's,#!include auth-passwdfile.conf.ext,#!include auth-system.conf.ext,' \
	/etc/dovecot/conf.d/10-auth.conf
RUN install -m 640 -o dovecot -g mail /dev/null /etc/dovecot/users
# Set default mail location to "/var/lib/mail"
RUN sed -i -e 's,#mail_location =,mail_location = /var/lib/mail/%n,' \
	/etc/dovecot/conf.d/10-mail.conf

# Mail storage directory, TLS key directory & Dovecot socket directory
VOLUME /var/lib/mail /etc/ssl/dovecot /run/dovecot

WORKDIR /home/
RUN echo "#!/bin/bash" >> run.sh
RUN echo "./tools --dep -p dovecot -o \" -F\"  -w 20 &> out.txt " >> run.sh
RUN echo "cat /root/${APP_NAME}_output/${APP_NAME}.json " >> run.sh
RUN chmod +x run.sh

CMD ["./run.sh"]