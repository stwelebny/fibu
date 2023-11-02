// Function to load content from a URL into a container

function submitForm(form) { 
     client = document.getElementById("mandant").value;
     if (!client) {
         alert('Bitte einen Mandanten angeben!');
         return;
    }
    let formData = {
        "Mandant": client,
        "Belegnummer": form.querySelector("[name='Belegnummer']").value,
        "Belegdatum": form.querySelector("[name='Belegdatum']").value,
        "Text": form.querySelector("[name='Text']").value,
        "SollKonto": form.querySelector("[name='SollKonto']").value,
        "HabenKonto": form.querySelector("[name='HabenKonto']").value,
        "Betrag": form.querySelector("[name='Betrag']").value
    };

    // Send data to server using Fetch API
    fetch('/cgi-bin/addBooking', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(formData)
    })
    .then(response => {
        if (!response.ok) { // If the HTTP status code is not in the 200-299 range
            // Try to parse the error message from the response
            return response.json().then(errData => {
                throw new Error(errData.message || 'Server error ' + response.status);
            });
        }
        return response.json(); // Process the response if it's ok
    })
    .then(data => {
        displaySubmissionResult(form, data);
    })
    .catch((error) => {
        // Pass only the error message to the displaySubmissionError function
        displaySubmissionError(form, error.message);
        console.error('Error:', error);
    });
}

function displaySubmissionResult(form, data) {
    const resultDiv = document.createElement('div');
    const bookingData = Array.from(form.elements).map(element => element.value).join(", ");

    resultDiv.innerText = bookingData + " OK";
    resultDiv.style.color = 'grey';

    form.parentNode.insertBefore(resultDiv, form.nextSibling);
}

function displaySubmissionError(form, error) {
    const errorDiv = document.createElement('div');
    errorDiv.innerText = error;
    errorDiv.style.color = 'red';

    form.parentNode.insertBefore(errorDiv, form.nextSibling);
}

function setupFormSubmission() {
    document.getElementById("bookingForm").addEventListener("submit", function(event) {
        event.preventDefault();
        submitForm(event.target);
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


function formatCurrencyValue(value) {
    return new Intl.NumberFormat('de-AT', {
        minimumFractionDigits: 2,
        maximumFractionDigits: 2
    }).format(value);
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
            <td style="text-align: right;">${formatCurrencyValue(entry["Soll"])}</td>
            <td style="text-align: right;">${formatCurrencyValue(entry["Haben"])}</td>
            <td>${entry.Text}</td>
        `;

        tableBody.appendChild(tableRow);
    });
}


function displayJournalData(data) {
    const tableBody = document.getElementById('journalDetails');
    tableBody.innerHTML = ''; // Clear any previous data

    data.forEach(entry => {
        const tableRow = document.createElement('tr');

        tableRow.innerHTML = `
            <td>${entry.Belegdatum}</td>
            <td>${entry.Belegnummer}</td>
            <td>${entry.SollKonto}</td>
            <td>${entry.HabenKonto}</td>
            <td>${entry.Text}</td>
            <td style="text-align: right;">${formatCurrencyValue(entry["Betrag"])}</td>
        `;

        tableBody.appendChild(tableRow);
    });
}

function displayJournalCheck(data) {
    const resultDiv = document.getElementById('journalCheckResult');
    resultDiv .innerHTML = data.message; 
}

function checkConsistency() {
    const client = getCookie('mandant');

    if (!client) {
        alert('Bitte einen Mandanten angeben!');
        return;
    }

    // Make an AJAX call to the CGI script
    fetch(`./cgi-bin/verifyJournal?client=${client}`)
        .then(response => response.json())
        .then(data => {
            displayJournalCheck(data);
        })
        .catch(error => {
            console.error('Fehler bei der Server-Abfrage', error);
        });
}



function fetchJournalData() {
    const client = getCookie('mandant');

    if (!client) {
        alert('Bitte einen Mandanten angeben!');
        return;
    }

    // Make an AJAX call to the CGI script
    fetch(`./cgi-bin/journalReport?client=${client}`)
        .then(response => response.json())
        .then(data => {
            displayJournalData(data);
        })
        .catch(error => {
            console.error('Fehler bei der Journalabfrage', error);
        });
}



function fetchBalanceList() {

    const client = getCookie('mandant');
    if (!client) {
        alert('Bitte einen Mandanten angeben!');
        return;
    }

    // Make an AJAX call to the CGI script
    fetch(`./cgi-bin/balanceList?client=${client}`)
        .then(response => response.json())
        .then(data => {
            displayBalanceList(data);
        })
        .catch(error => {
            console.error('Fehler bei der Abfrage', error);
        });
}

function displayBalanceList(data) {
    const tableBody = document.getElementById('balanceListDetails');
    tableBody.innerHTML = ''; // Clear any previous data

    data.forEach(entry => {
        const tableRow = document.createElement('tr');

        tableRow.innerHTML = `
            <td>${entry.Konto}</td>
            <td></td>
            <td style="text-align: right;">${formatCurrencyValue(entry["Soll-Saldo"])}</td>
            <td style="text-align: right;">${formatCurrencyValue(entry["Haben-Saldo"])}</td>
        `;

        tableBody.appendChild(tableRow);
    });
}


function handleCSVImport() {
    const fileInput = document.getElementById('csvFileInput');
    const file = fileInput.files[0];

    if (!file) {
        alert('Please select a CSV file to import.');
        return;
    }

    const reader = new FileReader();
    reader.readAsText(file);

    reader.onload = function(event) {
        const csvData = event.target.result;
        const rows = csvData.split('\n')
            .filter(row => row.trim().length > 0);
        const bookings = rows.map(row => {
            const columns = row.split(',').map(column => {
                return column.replace(/^"|"$/g, ''); // Remove quotes if they exist
            });
            return {
                belegnummer: columns[0],
                belegdatum: columns[1],
                text: columns[2],
                sollkonto: columns[3],
                habenkonto: columns[4],
                betrag: columns[5]
            };
        });

        for (const booking of bookings) {
            const form = createBookingForm(booking);
            form.addEventListener('submit', function(event) {
                console.log("Form submit event listener triggered.");
                event.preventDefault();
                event.stopPropagation();
                submitForm(event.target);
            });
            document.getElementById('multiBookingFormsContainer').appendChild(form);
        }
        console.log(document.querySelectorAll('.bookingForm').length + " forms appended.");
    };
}

let formCount = 0; // Global counter to track form IDs

function createBookingForm(booking) {
    const form = document.createElement('form');
    form.classList.add('bookingForm');
    form.id = 'bookingForm-' + formCount;
    formCount++;

    const fields = [
        { label: 'Belegnummer', name: 'belegnummer', type: 'text', value: booking.belegnummer },
        { label: 'Belegdatum', name: 'belegdatum', type: 'date', value: booking.belegdatum },
        { label: 'Text', name: 'text', type: 'text', value: booking.text },
        { label: 'SollKonto', name: 'sollkonto', type: 'text', value: booking.sollkonto },
        { label: 'HabenKonto', name: 'habenkonto', type: 'text', value: booking.habenkonto },
        { label: 'Betrag', name: 'betrag', type: 'number', value: booking.betrag }
    ];

    for (const field of fields) {
        const label = document.createElement('label');
        label.innerText = field.label;
        label.htmlFor = field.name + "-" + formCount;

        const input = document.createElement('input');
        input.id = field.name + "-" + formCount;
        input.name = field.label;
        input.type = field.type;
        input.value = field.value;

        form.appendChild(label);
        form.appendChild(input);
    }

    const submitButton = document.createElement('input');
    submitButton.type = 'submit';
    submitButton.value = 'Buchen';
    submitButton.addEventListener('click', function(event) {
      event.preventDefault();
      console.log("Submit button clicked for form:", form.id);
      submitForm(form);
    });
    form.appendChild(submitButton);

    return form;
}

async function submitForms() {
    const forms = document.querySelectorAll('[id^="bookingForm-"]');

    for (const form of forms) {
        await submitForm(form);
    }
}




