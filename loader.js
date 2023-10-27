// Function to load content from a URL into a container
function loadContent(url, containerId) {
    fetch(url)
        .then(response => response.text())
        .then(content => {
            document.getElementById(containerId).innerHTML = content;
        })
        .catch(error => {
            console.error('Error loading content:', error);
        });
}

// Load the menu and default content on page load
window.onload = () => {
    loadContent('menu.html', 'menuContainer');

    // Load default content or based on user navigation
    loadContent('bookingform.html', 'contentContainer');
};

// Later, you can expand this to handle menu clicks and load different content

