<template>
    <div class="container">
        <h1 class='welcome'>Welcome</h1>
        <Login />
    </div>
</template>


<script setup>
    import { onMounted } from 'vue'
    import { isAuthenticated } from '@/auth'
    import { useRouter } from 'vue-router'
    import Login from '../components/Login.vue'
    import { login } from '@/auth'

    const router = useRouter()

    onMounted(() => {
        if (isAuthenticated.value) {
            login(localStorage.getItem('token'), router)
        }
    })
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
