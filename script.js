// =====================================
// Fault-Tolerant Embedded Runtime System
// script.js
// =====================================

// Card Hover Animation
const cards = document.querySelectorAll(".card");

cards.forEach(card => {
    card.addEventListener("mouseenter", () => {
        card.style.background = "#334155";
        card.style.transform = "translateY(-8px)";
    });

    card.addEventListener("mouseleave", () => {
        card.style.background = "#1e293b";
        card.style.transform = "translateY(0px)";
    });
});

// Architecture Hover Animation
const architecture = document.querySelectorAll(".architecture div");

architecture.forEach(item => {
    item.addEventListener("mouseenter", () => {
        item.style.background = "#334155";
        item.style.transform = "scale(1.04)";
    });

    item.addEventListener("mouseleave", () => {
        item.style.background = "#1e293b";
        item.style.transform = "scale(1)";
    });
});

// Smooth Fade-In Animation
const observer = new IntersectionObserver((entries) => {
    entries.forEach(entry => {
        if (entry.isIntersecting) {
            entry.target.style.opacity = "1";
            entry.target.style.transform = "translateY(0)";
        }
    });
}, {
    threshold: 0.15
});

document.querySelectorAll(".section").forEach(section => {
    section.style.opacity = "0";
    section.style.transform = "translateY(40px)";
    section.style.transition = "all 0.8s ease";
    observer.observe(section);
});

// Console Message
console.log("====================================");
console.log("Fault-Tolerant Embedded Runtime System");
console.log("Developed by BIKASH SWAIN");
console.log("ESP32 • Embedded Systems • Runtime Safety");
console.log("====================================");
