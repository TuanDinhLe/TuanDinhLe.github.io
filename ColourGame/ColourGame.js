var numSquares = 6;
var colour = generateRandomColour(numSquares);
var squares = document.querySelectorAll(".square");
var title = document.getElementById("Display");
var message = document.querySelector("#Message");
var h1 = document.querySelector("h1");
var picked = pickedColour();
var modeButtons = document.querySelectorAll(".mode");
var resetButton = document.querySelector("#reset");


for (var i = 0; i < modeButtons.length; i++) {
	modeButtons[i].addEventListener("click", function() {
		modeButtons[0].classList.remove("selected");
		modeButtons[1].classList.remove("selected");
		this.classList.add("selected");
		this.textContent === "Easy" ? numSquares = 3: numSquares = 6;
		reset();
	})
}

title.textContent = picked;

for (var i = 0; i < squares.length; i++){
	squares[i].style.backgroundColor = colour[i];
	squares[i].addEventListener("click", function(){
		var pickedColour = this.style.backgroundColor;
		if(pickedColour === picked){
			message.textContent = "Correct!";
			resetButton.textContent = "You're good! Wanna play again?";
			changeColour(pickedColour);
			h1.style.backgroundColor = pickedColour;
		}
		else{
			this.style.backgroundColor = "#232323";
			message.textContent = "Try Again!";
		}
	})
}

resetButton.addEventListener("click", function(){
	reset();
})

function reset() {
	colour = generateRandomColour(numSquares);
	picked = pickedColour();
	title.textContent = picked;
	resetButton.textContent = "New Colour";
	for (var i = 0; i < squares.length; i++){
		if (colour[i]) {
			squares[i].style.display = "block"
			squares[i].style.backgroundColor = colour[i];
		} else {
			squares[i].style.display = "none";
		}
	}
	h1.style.backgroundColor = "steelblue";
	message.textContent = "";
}



function changeColour(colour){
	for (var i = 0; i < squares.length; i++) {
	squares[i].style.backgroundColor = colour;
	}
}

function pickedColour(){
	var index = Math.floor(Math.random() * (colour.length));
	return colour[index];
}

function generateRandomColour(num){
	var arr = [];

	for (var i = 0; i < num; i++) {
		arr.push(randomColour());
	}

	return arr;
}

function randomColour(){
	var r = Math.floor(Math.random() * 256);
	var g = Math.floor(Math.random() * 256);
	var b = Math.floor(Math.random() * 256);
	return "rgb(" + r + ", " + g + ", " + b + ")";
}

