<template>
    <div style="display: flex; flex-direction: column;">
        <p>User Actions</p>
        <button @click="router.push('/leave-room')" class="button-gray action">Leave Room</button>
    </div>
</template>


<script setup>
    import { useRouter } from 'vue-router'
    const router = useRouter()
</script>


<style scoped>
.action {
    margin-top: 40px;
    font-size: 18pt;
    min-width: 180px;
    min-height: 50px;
}

</style>
