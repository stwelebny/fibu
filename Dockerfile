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


# Enable CGI and copy Apache configurations
COPY ./httpd.conf /usr/local/apache2/conf/httpd.conf

# Copy C++ CGI script and compile it
COPY ./addBooking.cpp /usr/local/apache2/cgi-bin/
RUN g++ -std=c++17 /usr/local/apache2/cgi-bin/addBooking.cpp -o /usr/local/apache2/cgi-bin/addBooking -lcgicc -ljsoncpp

# Make the CGI script executable
RUN chmod +x /usr/local/apache2/cgi-bin/addBooking

# Expose HTTP and HTTPS ports
EXPOSE 80
EXPOSE 443

# Start Apache in the foreground
CMD ["httpd-foreground"]

