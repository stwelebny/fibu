# Start from a base Linux image with Apache
FROM httpd:2.4

# Update and install necessary tools and libraries
RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y iptables && \
    apt-get clean

# Set up basic firewall rules with iptables
# Adjust these as per your needs
RUN iptables -A INPUT -i lo -j ACCEPT
RUN iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
RUN iptables -A INPUT -p tcp --dport 80 -j ACCEPT
RUN iptables -A INPUT -p tcp --dport 443 -j ACCEPT
RUN iptables -A INPUT -j DROP

# For DDoS protection, you'd typically rely on more comprehensive tools or services.
# For a basic layer of protection, you can use iptables to limit the number of connections:
RUN iptables -A INPUT -p tcp --dport 80 -m limit --limit 50/minute --limit-burst 200 -j ACCEPT

# Copy your web app's files to the Apache document root
# COPY ./your-web-app-directory/ /usr/local/apache2/htdocs/

# Expose HTTP and HTTPS ports
EXPOSE 80
EXPOSE 443

# Start Apache in the foreground
CMD ["httpd-foreground"]

