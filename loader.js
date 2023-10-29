// Function to load content from a URL into a container

function setupFormSubmission() {
    document.getElementById("bookingForm").addEventListener("submit", function(event) {
        event.preventDefault(); // Prevent default form submission

         client = document.getElementById("mandant").value;
         if (!client) {
             alert('Bitte einen Mandanten angeben!');
             return;
        }
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
               if (typeof callback === 'function') {
                    callback();
                } else {
                    console.error(`Callback function not found or not a function.`);
                }
            }
        })
        .catch(error => {
            console.error('Error loading content:', error);
        });
}

function setCookie(name, value, days) {
    let expires = "";
    if (days) {
        const date = new Date();
        date.setTime(date.getTime() + (days * 24 * 60 * 60 * 1000));
        expires = "; expires=" + date.toUTCString();
    }
    document.cookie = name + "=" + (value || "") + expires + "; path=/";
}

function getCookie(name) {
    const value = "; " + document.cookie;
    const parts = value.split("; " + name + "=");
    if (parts.length === 2) return parts.pop().split(";").shift();
}


function setupMenuNavigation() {
    const links = document.querySelectorAll('.dynamic-link');
    links.forEach(link => {
        link.addEventListener('click', function(event) {
            console.log('Link clicked'); // Add this line
            event.preventDefault(); // Prevent the default navigation
            const contentUrl = this.getAttribute('href');
            const callBackName = this.getAttribute('callBack'); // Fixed the typo here
            if (callBackName && callBackName !="")
                callBack = eval(callBackName)
            else
                callBack = null;
            if (contentUrl && contentUrl !== '#') {
                loadContent(contentUrl, 'contentContainer', callBack);
            }
        });
    });
    const mandantInput = document.getElementById('mandant');

    mandantInput.addEventListener('change', function() {
        setCookie('mandant', this.value, 365); // Here, the cookie will expire after 365 days. Adjust as needed.
    });

    const mandantValue = getCookie('mandant');
    if (mandantValue) {
        mandantInput.value = mandantValue;
    }


}


// Load the menu and default content on page load
window.onload = () => {
    loadContent('menu.html', 'menuContainer', setupMenuNavigation);
    // Load default content or based on user navigation
    loadContent('bookingform.html', 'contentContainer', setupFormSubmission);
};

function fetchAccountData() {
    const accountNumber = document.getElementById('accountNumber').value;
    const client = getCookie('mandant');
    if (!accountNumber) {
        alert('Bitte eine Kontonummer angeben!');
        return;
    }

    if (!client) {
        alert('Bitte einen Mandanten angeben!');
        return;
    }

    // Make an AJAX call to the CGI script
    fetch(`./cgi-bin/account?client=${client}&accountnumber=${accountNumber}`)
        .then(response => response.json())
        .then(data => {
            displayData(data);
        })
        .catch(error => {
            console.error('Fehler bei der Kontoabfrage', error);
        });
}

function displayData(data) {
    const tableBody = document.getElementById('accountDetails');
    tableBody.innerHTML = ''; // Clear any previous data

    data.forEach(entry => {
        const tableRow = document.createElement('tr');

        tableRow.innerHTML = `
            <td>${entry.Belegdatum}</td>
            <td>${entry.Belegnummer}</td>
            <td>${entry.Gegenkonto}</td>
            <td>${entry.Soll || 'N/A'}</td>
            <td>${entry.Haben || 'N/A'}</td>
            <td>${entry.Text}</td>
        `;

        tableBody.appendChild(tableRow);
    });
}

