<template>
    <div class="container">
        <h1 class='welcome'>Welcome</h1>
        <Auth />
    </div>
</template>


<script setup>
    import Auth from '../components/Auth.vue'
    import { ref } from 'vue'
</script>


<style scoped>
    .container {
        display: flex;
        flex-direction: column;
        justify-content: center;
        align-items: center;
        text-align: center;
    }

    .welcome {
        font-size: 40pt;
        margin-top: 40px;
        margin-bottom: 80px;
    }
</style>
