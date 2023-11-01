# Start from a base Linux image with Apache
FROM httpd:2.4

# Update and install necessary tools and libraries
RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y iptables g++ build-essential libcgicc-dev libjsoncpp-dev && \
    apt-get clean

RUN find / -name json.h

VOLUME /fibudata

COPY *.html /usr/local/apache2/htdocs/
COPY *.js /usr/local/apache2/htdocs/
COPY *.css /usr/local/apache2/htdocs/
COPY EinheitskontenrahmenOE.txt /app/

# Enable CGI and copy Apache configurations
COPY ./httpd.conf /usr/local/apache2/conf/httpd.conf

# Copy C++ CGI script and compile it
COPY ./addBooking.cpp /usr/local/apache2/cgi-bin/
COPY ./account.cpp /usr/local/apache2/cgi-bin/
COPY ./balanceList.cpp /usr/local/apache2/cgi-bin/
COPY ./balanceReport.cpp /usr/local/apache2/cgi-bin/
COPY ./journalReport.cpp /usr/local/apache2/cgi-bin/

RUN g++ -std=c++17 /usr/local/apache2/cgi-bin/addBooking.cpp -o /usr/local/apache2/cgi-bin/addBooking -lcgicc -ljsoncpp
RUN g++ -std=c++17 /usr/local/apache2/cgi-bin/account.cpp -o /usr/local/apache2/cgi-bin/account -lcgicc -ljsoncpp
RUN g++ -std=c++17 /usr/local/apache2/cgi-bin/balanceList.cpp -o /usr/local/apache2/cgi-bin/balanceList -lcgicc -ljsoncpp
RUN g++ -std=c++17 /usr/local/apache2/cgi-bin/balanceReport.cpp -o /usr/local/apache2/cgi-bin/balanceReport -lcgicc -ljsoncpp
RUN g++ -std=c++17 /usr/local/apache2/cgi-bin/journalReport.cpp -o /usr/local/apache2/cgi-bin/journalReport -lcgicc -ljsoncpp

# Make the CGI script executable
RUN chmod +x /usr/local/apache2/cgi-bin/addBooking
RUN chmod +x /usr/local/apache2/cgi-bin/account
RUN chmod +x /usr/local/apache2/cgi-bin/balanceList
RUN chmod +x /usr/local/apache2/cgi-bin/balanceReport

# Expose HTTP and HTTPS ports
EXPOSE 80
EXPOSE 443

# Start Apache in the foreground
CMD ["httpd-foreground"]

