<script setup>
import Home from './components/Home.vue'
import SignUp from './components/SignUp.vue'
import Login from './components/Login.vue'
</script>

<template>
    <SignUp />
    <!--
    <div class="parent-container">
        <SignUp />
    </div>
    -->
</template>

<style>
.parent-container {
    height: 100vh;
    width: 100%;
    display: flex;
    justify-content: center;
    align-items: center;
}
</style>
