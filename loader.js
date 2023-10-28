// Function to load content from a URL into a container

function setupFormSubmission() {
    document.getElementById("bookingForm").addEventListener("submit", function(event) {
        event.preventDefault(); // Prevent default form submission

        // Extract values from form
        let formData = {
            "Mandant": document.getElementById("mandant").value,
            "Belegnummer": document.getElementById("belegnummer").value,
            "Belegdatum": document.getElementById("belegdatum").value,
            "Text": document.getElementById("text").value,
            "SollKonto": document.getElementById("sollkonto").value,
            "HabenKonto": document.getElementById("habenkonto").value,
            "Betrag": document.getElementById("betrag").value
        };

        // Send data to server using Fetch API
        fetch('/cgi-bin/addBooking', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(formData)
        })
        .then(response => response.json())
        .then(data => {
            alert(data.message); // Display success/error message
        })
        .catch((error) => {
            console.error('Error:', error);
            alert('An error occurred. Please try again.');
        });
    });
}

function loadContent(url, containerId, callback = null) {
    fetch(url)
        .then(response => response.text())
        .then(content => {
            document.getElementById(containerId).innerHTML = content;
            if (callback) {
                callback();
            }
        })
        .catch(error => {
            console.error('Error loading content:', error);
        });
}

// Load the menu and default content on page load
window.onload = () => {
    loadContent('menu.html', 'menuContainer');

    // Load default content or based on user navigation
    loadContent('bookingform.html', 'contentContainer', setupFormSubmission);
};

// Later, you can expand this to handle menu clicks and load different content

