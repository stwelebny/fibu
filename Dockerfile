# Start from a base Linux image with Apache
FROM httpd:2.4

# Update and install necessary tools and libraries
RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y iptables g++ build-essential libcgicc-dev libjsoncpp-dev openssl libssl-dev openssh-server && \
    apt-get clean

RUN find / -name json.h

VOLUME /fibudata

COPY *.html /usr/local/apache2/htdocs/
COPY *.js /usr/local/apache2/htdocs/
COPY *.css /usr/local/apache2/htdocs/
COPY EinheitskontenrahmenOE.txt /app/

# Enable CGI and copy Apache configurations
COPY ./httpd.conf /usr/local/apache2/conf/httpd.conf


# Create an Apache configuration file for basic auth

RUN echo '<Directory "/usr/local/apache2/htdocs/">' >> /usr/local/apache2/conf/extra/httpd-basic-auth.conf && \
    echo '    AuthType Basic' >> /usr/local/apache2/conf/extra/httpd-basic-auth.conf && \
    echo '    AuthName "Restricted Area"' >> /usr/local/apache2/conf/extra/httpd-basic-auth.conf && \
    echo '    AuthUserFile "/fibudata/.htpasswd"' >> /usr/local/apache2/conf/extra/httpd-basic-auth.conf && \
    echo '    Require valid-user' >> /usr/local/apache2/conf/extra/httpd-basic-auth.conf && \
    echo '</Directory>' >> /usr/local/apache2/conf/extra/httpd-basic-auth.conf

# Ensure httpd.conf includes the basic auth configuration
RUN echo 'Include conf/extra/httpd-basic-auth.conf' >> /usr/local/apache2/conf/httpd.conf

# Copy C++ CGI script and compile it
COPY ./addBooking.cpp /usr/local/apache2/cgi-bin/
COPY ./account.cpp /usr/local/apache2/cgi-bin/
COPY ./balanceList.cpp /usr/local/apache2/cgi-bin/
COPY ./balanceReport.cpp /usr/local/apache2/cgi-bin/
COPY ./journalReport.cpp /usr/local/apache2/cgi-bin/
COPY ./hash.h /usr/local/apache2/cgi-bin/
COPY ./verifyJournal.cpp /usr/local/apache2/cgi-bin/
COPY ./logger.h /usr/local/apache2/cgi-bin/

RUN g++ -std=c++17 /usr/local/apache2/cgi-bin/addBooking.cpp -o /usr/local/apache2/cgi-bin/addBooking -lcgicc -ljsoncpp -lssl -lcrypto
RUN g++ -std=c++17 /usr/local/apache2/cgi-bin/account.cpp -o /usr/local/apache2/cgi-bin/account -lcgicc -ljsoncpp
RUN g++ -std=c++17 /usr/local/apache2/cgi-bin/balanceList.cpp -o /usr/local/apache2/cgi-bin/balanceList -lcgicc -ljsoncpp
RUN g++ -std=c++17 /usr/local/apache2/cgi-bin/balanceReport.cpp -o /usr/local/apache2/cgi-bin/balanceReport -lcgicc -ljsoncpp
RUN g++ -std=c++17 /usr/local/apache2/cgi-bin/journalReport.cpp -o /usr/local/apache2/cgi-bin/journalReport -lcgicc -ljsoncpp
RUN g++ -std=c++17 /usr/local/apache2/cgi-bin/verifyJournal.cpp -o /usr/local/apache2/cgi-bin/verifyJournal -lcgicc -ljsoncpp -lssl -lcrypto

# Make the CGI script executable
RUN chmod +x /usr/local/apache2/cgi-bin/addBooking
RUN chmod +x /usr/local/apache2/cgi-bin/account
RUN chmod +x /usr/local/apache2/cgi-bin/balanceList
RUN chmod +x /usr/local/apache2/cgi-bin/balanceReport
RUN chmod +x /usr/local/apache2/cgi-bin/journalReport
RUN chmod +x /usr/local/apache2/cgi-bin/verifyJournal

# Expose HTTP and HTTPS ports
EXPOSE 80
EXPOSE 443

# Start Apache in the foreground
CMD ["httpd-foreground"]

