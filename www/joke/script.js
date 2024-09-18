const jokeDisplay = document.getElementById('jokeDisplay');

// Function to fetch a dad joke from an API
async function fetchJoke() {
    try {
        const response = await fetch('https://icanhazdadjoke.com/', {
            headers: {
                'Accept': 'application/json'
            }
        });
        const data = await response.json();
        return data.joke;
    } catch (error) {
        console.log('Error fetching joke:', error);
        return 'Error fetching joke. Please try again later.';
    }
}

// Function to display a new joke
async function displayJoke() {
    const joke = await fetchJoke();
    jokeDisplay.textContent = joke;
}

// Display an initial joke when the page loads
displayJoke();

// Automatically fetch and display a new joke every 20 seconds
setInterval(displayJoke, 20000);
