<template>
    <div class="parent">
        <h1 class='welcome'>Welcome</h1>
        <Login />
        <img src="../assets/positivessl_trust_seal_md_167x42.png" style="margin-top: 40px;">
    </div>
</template>


<script setup>
    import { onMounted } from 'vue'
    import { isAuthenticated } from '@/auth'
    import { useRouter } from 'vue-router'
    import Login from '../components/Login.vue'
    import { login, verifyToken } from '@/auth'

    const router = useRouter()

    onMounted( async () => {
        const token = localStorage.getItem('token')
            
        if (token) {
            await verifyToken()
        }

        if (isAuthenticated.value) {
            login(token, router)
        }
    })
</script>


<style scoped>
.parent {
    display: flex;
    flex-direction: column;
    align-items: center;
}

.welcome {
    font-size: 40pt;
    margin-top: 15px;
    margin-bottom: 40px;
}

</style>
